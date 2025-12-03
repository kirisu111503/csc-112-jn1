%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

typedef struct errorList{
   int line_error;
   char *error_type;
   struct errorList *next;
}errorList;

typedef struct vars{
    char *id;
    int data_type;  // 'i' = int, 'c' = char, 's' = string
    union {
        int val;
        char *str_val;
    } data;
    struct vars *next;
} vars;

vars *headVars = NULL;
vars *tailVars = NULL;
errorList *headErrList = NULL;
errorList *tailErrList = NULL;

char *currentDataType = NULL;
char *currentVarBeingDeclared = NULL;
int hasError = 0;

extern int yylineno;

void yyerror(const char *fmt, ...);
void printErrorTable();
void cleanupErrorTable();
int getVariableValue(char *variableName);
void cleanupVariableTable();
const char* typeName(int dt);
vars* getVariable(char* variable);
void createVariable(char *DTYPE, char* variable, int val, char *str_val);
void variableReAssignment(char* variable, int val, char *str_val);
void printVariableTable();
%}

%code requires {
    typedef struct {
        int type;     // 1=string, 2=char literal, 3=numeric (int/char/var/literal), 4=error
        char* text;   // printable representation
    } Disp;
}

%union {
    int num;
    char character;
    char *str;
    Disp disp;
}

%token <str> DATA_TYPE VARIABLE STRING
%token ASSIGNMENT DISPLAY COMMA SEMI
%token <num> INTEGER
%token <character> CHARACTER

%right ASSIGNMENT
%left '+' '-'
%left '*' '/'
%precedence UMINUS

%type <num> expression
%type <disp> display_arg display_expr
%type <str> data_type

%%

program: statement_list ;

statement_list:
    statement_list statement
  | /* empty */
  ;

statement:
    display_statement
  | declaration_statement
  | assignment_statement
  | error SEMI { yyerrok; }
  ;

/* ============================ DISPLAY ============================ */

display_statement:
    DISPLAY '(' display_arg ')' SEMI
    {
        if (hasError || $3.type == 4) {
            if ($3.text) free($3.text);
            YYABORT;
        }
        if ($3.text) {
            printf("LINE %d: %s\n", yylineno, $3.text);
            free($3.text);
        }
    }
    ;

display_arg: display_expr { $$ = $1; }

display_expr:
    STRING
    {
        $$.type = 1;
        $$.text = strdup($1);
        free($1);
    }
  | CHARACTER
    {
        $$.type = 2;
        $$.text = malloc(2);
        if ($$.text) { $$.text[0] = $1; $$.text[1] = '\0'; }
    }
  | INTEGER
    {
        $$.type = 3;
        $$.text = malloc(20);
        if ($$.text) sprintf($$.text, "%d", $1);
    }
  | VARIABLE
    {
        vars *v = getVariable($1);
        if (!v) {
            yyerror("Undefined variable '%s' in display on line %d", $1, yylineno);
            $$.type = 4; $$.text = NULL;
            free($1);
            YYABORT;
        }
        if (v->data_type == 's') {
            $$.type = 1;
            $$.text = strdup(v->data.str_val ? v->data.str_val : "");
        } else {
            $$.type = 3;
            $$.text = malloc(20);
            if ($$.text) sprintf($$.text, "%d", v->data.val);
        }
        free($1);
    }
  /* String concatenation only (+ requires at least one string operand) */
  | display_expr '+' display_expr
    {
        if (hasError || $1.type == 4 || $3.type == 4) goto concat_err;

        if ($1.type != 1 && $3.type != 1) {
            yyerror("Inside display(), '+' can only be used for string concatenation. "
                    "Numeric addition is not allowed on line %d", yylineno);
            goto concat_err;
        }

        size_t len = strlen($1.text) + strlen($3.text) + 1;
        $$.text = malloc(len);
        $$.type = 1;
        if (!$$.text) {
            yyerror("Memory allocation failed during concatenation in display on line %d", yylineno);
            goto concat_err;
        }
        strcpy($$.text, $1.text);
        strcat($$.text, $3.text);

        free($1.text);
        free($3.text);
        goto concat_end;

    concat_err:
        if ($1.text) free($1.text);
        if ($3.text) free($3.text);
        $$.type = 4;
        $$.text = NULL;
        YYABORT;

    concat_end: ;
    }
  /* Forbid all other operators inside display */
  | display_expr '-' display_expr
    {
        yyerror("Operator '-' is not allowed inside display() on line %d", yylineno);
        if ($1.text) free($1.text);
        if ($3.text) free($3.text);
        $$.type = 4; $$.text = NULL;
        YYABORT;
    }
  | display_expr '*' display_expr
    {
        yyerror("Operator '*' is not allowed inside display() on line %d", yylineno);
        if ($1.text) free($1.text);
        if ($3.text) free($3.text);
        $$.type = 4; $$.text = NULL;
        YYABORT;
    }
  | display_expr '/' display_expr
    {
        yyerror("Operator '/' is not allowed inside display() on line %d", yylineno);
        if ($1.text) free($1.text);
        if ($3.text) free($3.text);
        $$.type = 4; $$.text = NULL;
        YYABORT;
    }
  | '-' display_expr %prec UMINUS
    {
        yyerror("Unary minus is not allowed inside display() on line %d", yylineno);
        if ($2.text) free($2.text);
        $$.type = 4; $$.text = NULL;
        YYABORT;
    }
  | '(' display_expr ')'
    { $$ = $2; }  /* allow grouping only */
    ;

/* ============================ DECLARATION & ASSIGNMENT ============================ */

declaration_statement:
    data_type declaration_list SEMI
    {
        if (currentDataType) { free(currentDataType); currentDataType = NULL; }
        if (currentVarBeingDeclared) { free(currentVarBeingDeclared); currentVarBeingDeclared = NULL; }
        if (hasError) YYABORT;
    }

data_type: DATA_TYPE { currentDataType = strdup($1); $$ = currentDataType; }

declaration_list:
    declaration_list COMMA var_decl
  | var_decl
  ;

var_decl:
    VARIABLE
    {
        if (strcmp(currentDataType, "string") == 0)
            createVariable(currentDataType, $1, 0, "");
        else
            createVariable(currentDataType, $1, 0, NULL);
        free($1);
    }
  | VARIABLE ASSIGNMENT expression
    {
        if (strcmp(currentDataType, "string") == 0) {
            yyerror("Cannot assign expression to string variable '%s' on line %d", $1, yylineno);
            free($1);
            YYABORT;
        }
        createVariable(currentDataType, $1, $3, NULL);
        free($1);
    }
  | VARIABLE ASSIGNMENT STRING
    {
        if (strcmp(currentDataType, "string") != 0) {
            yyerror("Cannot assign string literal to non-string variable '%s' on line %d", $1, yylineno);
            free($1); free($3);
            YYABORT;
        }
        createVariable(currentDataType, $1, 0, $3);
        free($1); free($3);
    }
  | VARIABLE ASSIGNMENT CHARACTER
    {
        if (strcmp(currentDataType, "string") == 0) {
            yyerror("Cannot assign char literal to string variable '%s' on line %d", $1, yylineno);
            free($1);
            YYABORT;
        }
        createVariable(currentDataType, $1, (int)$3, NULL);
        free($1);
    }
  ;

assignment_statement:
    assignment_list SEMI { if (hasError) YYABORT; }

assignment_list:
    assignment_list COMMA assignment
  | assignment
  ;

assignment:
    VARIABLE ASSIGNMENT expression
    {
        variableReAssignment($1, $3, NULL);
        free($1);
    }
  | VARIABLE ASSIGNMENT STRING
    {
        vars *v = getVariable($1);
        if (!v) { yyerror("Undefined variable '%s' on line %d", $1, yylineno); free($1); free($3); YYABORT; }
        if (v->data_type != 's') { yyerror("Cannot assign string to non-string variable '%s' on line %d", $1, yylineno); free($1); free($3); YYABORT; }
        variableReAssignment($1, 0, $3);
        free($1); free($3);
    }
  | VARIABLE ASSIGNMENT CHARACTER
    {
        vars *v = getVariable($1);
        if (!v) { yyerror("Undefined variable '%s' on line %d", $1, yylineno); free($1); YYABORT; }
        if (v->data_type == 's') { yyerror("Cannot assign char to string variable '%s' on line %d", $1, yylineno); free($1); YYABORT; }
        variableReAssignment($1, (int)$3, NULL);
        free($1);
    }
  | VARIABLE ASSIGNMENT VARIABLE
    {
        vars *t = getVariable($1);
        vars *s = getVariable($3);
        if (!t || !s) {
            yyerror("Undefined variable in assignment on line %d", yylineno);
            free($1); free($3);
            YYABORT;
        }
        if (t->data_type == 's' && s->data_type != 's') {
            yyerror("Cannot assign non-string to string variable '%s' on line %d", $1, yylineno);
            free($1); free($3);
            YYABORT;
        }
        if (t->data_type != 's' && s->data_type == 's') {
            yyerror("Cannot assign string to non-string variable '%s' on line %d", $1, yylineno);
            free($1); free($3);
            YYABORT;
        }
        if (t->data_type == 's')
            variableReAssignment($1, 0, s->data.str_val);
        else
            variableReAssignment($1, s->data.val, NULL);
        free($1); free($3);
    }
  ;

/* Full arithmetic expressions – allowed outside display */
expression:
    INTEGER               { $$ = $1; }
  | CHARACTER             { $$ = (int)$1; }
  | VARIABLE
    {
        if (currentVarBeingDeclared && strcmp($1, currentVarBeingDeclared) == 0) {
            yyerror("Variable '%s' used in its own initializer on line %d", $1, yylineno);
            free($1);
            $$ = 0;
            YYABORT;
        }
        $$ = getVariableValue($1);
        free($1);
    }
  | '(' expression ')'    { $$ = $2; }
  | expression '+' expression { $$ = $1 + $3; }
  | expression '-' expression { $$ = $1 - $3; }
  | expression '*' expression { $$ = $1 * $3; }
  | expression '/' expression
    {
        if ($3 == 0) { yyerror("Division by zero on line %d", yylineno); YYABORT; }
        $$ = $1 / $3;
    }
  | '-' expression %prec UMINUS { $$ = -$2; }
  | '+' expression %prec UMINUS { $$ = $2; }
  ;

%%

/* ====================== C SUPPORT FUNCTIONS (unchanged except minor fixes) ====================== */

int main(void) {
    printf("Welcome to my Custom sPyC!\n");
    int result = yyparse();
    if (headVars) printVariableTable();
    if (headErrList) printErrorTable();
    if (headErrList) cleanupErrorTable();
    if (headVars) cleanupVariableTable();
    return result;
}

void yyerror(const char *fmt, ...) {
    if (fmt && strcmp(fmt, "syntax error") == 0) return;
    hasError = 1;
    va_list args;
    va_start(args, fmt);
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    errorList *node = calloc(1, sizeof(errorList));
    if (!node) { fprintf(stderr, "Memory error\n"); return; }
    node->line_error = yylineno;
    node->error_type = strdup(buffer);
    node->next = NULL;
    if (!headErrList) headErrList = tailErrList = node;
    else { tailErrList->next = node; tailErrList = node; }
}

/* printErrorTable, cleanupErrorTable, getVariableValue, cleanupVariableTable, typeName, getVariable,
   createVariable, variableReAssignment, printVariableTable remain exactly as you had them */

int getVariableValue(char *name) {
    vars *v = getVariable(name);
    if (!v) {
        yyerror("Undefined variable '%s' on line %d", name, yylineno);
        return 0;
    }
    if (v->data_type == 's') {
        yyerror("Cannot perform arithmetic on string variable '%s' on line %d", name, yylineno);
        return 0;
    }
    return v->data.val;
}

const char* typeName(int dt) {
    return dt=='i' ? "int" : dt=='c' ? "char" : "string";
}

/* ... rest of your C functions (getVariable, createVariable, variableReAssignment, etc.)
   are unchanged and work perfectly with the new rules ... */



/*---------------------------------Error handling---------------------------------------------------*/
void yyerror(const char *fmt, ...) {
    if (fmt && strcmp(fmt, "syntax error") == 0)
        return;

    hasError = 1;

    va_list args;
    va_start(args, fmt);

    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    errorList *currentError = calloc(1, sizeof(errorList));
    if(!currentError){
        fprintf(stderr, "Memory allocation failed for error node. Parser at fault.\n");
        return;
    }

    currentError->line_error = yylineno;
    currentError->error_type = strdup(buffer);
    if(!currentError->error_type){
        fprintf(stderr, "Memory allocation failed for error message. Parser at fault.\n");
        free(currentError);
        return;
    }
    currentError->next = NULL;

    if(!headErrList){
        headErrList = tailErrList = currentError;
    }else{
        tailErrList->next = currentError;
        tailErrList = currentError; 
    }
}

void printErrorTable() {
    errorList *curr = headErrList;
    printf("=========== Error Table ============\n");
    while (curr) {
        printf("Line %d: %s", curr->line_error, curr->error_type);
        if (strlen(curr->error_type) == 0 || curr->error_type[strlen(curr->error_type) - 1] != '\n')
            printf("\n");
        curr = curr->next;
    }
    printf("===================================\n");
}



void cleanupErrorTable(){
    while (headErrList) {
        errorList *tmp = headErrList;
        headErrList = headErrList->next;
        free(tmp->error_type);
        free(tmp);
    }
    headErrList = tailErrList = NULL;
}


void cleanupVariableTable() {
    vars *current = headVars;
    vars *next;

    while (current) {
        next = current->next;
        if (current->data_type == 's' && current->data.str_val) free(current->data.str_val);
        free(current->id);
        free(current);
        current = next;
    }
    headVars = tailVars = NULL;
}

const char* typeName(char dt) {
    return (dt=='i')?"int":(dt=='c')?"char":"string";
}


vars* getVariable(char* variable) {
    vars *current = headVars;
    while (current) {
        if (strcmp(current->id, variable) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void createVariable(char *DTYPE, char* variable, int val, char *str_val) {  
    if(isdigit(variable[0])){
        yyerror("Variable %s can't start in INTEGER, in line %d.", variable, yylineno);
        hasError = 1;
        return;
    }

    vars *existing = getVariable(variable);

    if (existing && DTYPE && strlen(DTYPE) > 0) {
        yyerror("Variable '%s' is already declared with type '%s' on line %d", 
                variable, typeName(existing->data_type), yylineno);
        hasError = 1;
        return;
    }

    if (existing && (!DTYPE || strlen(DTYPE) == 0)) {
        variableReAssignment(variable, val, str_val);
        return;
    }

    if (!existing && (!DTYPE || strlen(DTYPE) == 0)) {
        yyerror("Undefined variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }

    if (strcmp(DTYPE, "int") == 0 && str_val != NULL) {
        yyerror("Cannot assign string value to int variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }
    if (strcmp(DTYPE, "char") == 0 && str_val != NULL) {
        yyerror("Cannot assign string value to char variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }
    if (strcmp(DTYPE, "string") == 0 && str_val == NULL) {
        yyerror("Cannot assign non-string value to string variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }

    vars *newVar = calloc(1, sizeof(vars));
    if (!newVar) {
        fprintf(stderr, "Failed to allocate memory for variable '%s'\n", variable);
        return;
    }

    if (strcmp(DTYPE, "int") == 0) {
        newVar->data_type = 'i';
        newVar->data.val = val;
    } else if (strcmp(DTYPE, "char") == 0) {
        newVar->data_type = 'c';
        newVar->data.val = val;
    } else if (strcmp(DTYPE, "string") == 0) {
        newVar->data_type = 's';
        // For strings, use provided value or empty string
        if(str_val != NULL) {
            newVar->data.str_val = strdup(str_val);
        } else {
            newVar->data.str_val = strdup("");
        }
        if (!newVar->data.str_val) {
            fprintf(stderr, "Failed to allocate memory for string value\n");
            free(newVar);
            return;
        }
    } else {
        yyerror("Invalid data type '%s' for variable '%s'", DTYPE, variable);
        free(newVar);
        return;
    }

    newVar->id = strdup(variable);
    if (!newVar->id) {
        fprintf(stderr, "Failed to allocate memory for variable ID '%s'\n", variable);
        if (newVar->data_type == 's' && newVar->data.str_val)
            free(newVar->data.str_val);
        free(newVar);
        return;
    }

    newVar->next = NULL;

    if (!headVars) {
        headVars = tailVars = newVar;
    } else {
        tailVars->next = newVar;
        tailVars = newVar;
    }

    printf("Variable '%s' successfully created on line %d.\n", variable, yylineno);
}

void variableReAssignment(char* variable, int val, char *str_val){
    vars *existing = getVariable(variable);

    if(!existing){
        yyerror("Undefined variable %s on line %d.", variable, yylineno);
        hasError = 1;
        return;
    }

    if (existing->data_type == 'i' && str_val != NULL) {
        yyerror("Cannot assign string value to int variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }
    if (existing->data_type == 'c' && str_val != NULL) {
        yyerror("Cannot assign string value to char variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }
    if (existing->data_type == 's' && str_val == NULL) {
        yyerror("Cannot assign non-string value to string variable '%s' on line %d", variable, yylineno);
        hasError = 1;
        return;
    }

    if(existing->data_type == 'i' || existing->data_type == 'c'){
        existing->data.val = val;
    } else {
        char *new_str = strdup(str_val ? str_val : "");
        if(!new_str){
            yyerror("Failed to allocate memory for str value on line %d", yylineno);
            hasError = 1;
            return;
        }
        free(existing->data.str_val);
        existing->data.str_val = new_str;
    }
    
    printf("Variable '%s' updated successfully on line %d.\n", variable, yylineno);
}


void printVariableTable() {
    printf("\n=== Variable Table ===\n");
    vars *curr = headVars;
    while (curr) {
        printf("Variable: %s, Type: %s", curr->id, typeName(curr->data_type));
        if (curr->data_type == 's') {
            printf(", Value: \"%s\"\n", curr->data.str_val);
        } else {
            printf(", Value: %d\n", curr->data.val);
        }
        curr = curr->next;
    }
    printf("======================\n\n");
}