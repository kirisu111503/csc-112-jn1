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


// Symbol table structure
typedef struct vars{
    char *id;
    int data_type;
    union {
        int val;      // for int/char
        char *str_val; // for string
    } data;
    struct vars *next;
} vars;

typedef struct logs{
    char *info;
    int line;
    int isError;
    struct logs *next;
}logs;


vars *headVars = NULL;
vars *tailVars = NULL;
errorList *headErrList = NULL;
errorList *tailErrList = NULL;
logs *headLogs = NULL;
logs *tailLogs = NULL;
char *currentDataType = NULL;
char *currentVarBeingDeclared = NULL;  // Track variable being declared
int isRecovering = 0;
int hasError = 0;

extern int yylineno;
extern int yylex();
void yyerror(const char *fmt, ...);
void printErrorTable();
void cleanupErrorTable();
int getVariableValue(char *variableName);
void cleanupVariableTable();
const char* typeName(char dt);
vars* getVariable(char* variable);
void createVariable(char *DTYPE, char* variable, int val, char *str_val);
void variableReAssignment(char* variable, int val, char *str_val);
void printVariableTable();
%}

// Define Disp struct BEFORE %union so it's available in parser.tab.h
%code requires {
    typedef struct {
        int type;      // 1=string 2=char 3=int 4=id  
        char* text;    // always printable
    } Disp;
}

%union {
    int num;
    char character;
    char *str;
    Disp  disp;
}

%token <str> DATA_TYPE
%token <str> VARIABLE
%token ASSIGNMENT
%token DISPLAY
%token COMMA
%token DIGIT
%token SEMI
%token <str> STRING 
%token <num> INTEGER
%token <character> CHARACTER

// Operator precedence (lowest to highest)
%right ASSIGNMENT
%left '+' '-'
%left '*' '/'
%precedence UMINUS

%type <num> expression
%type <disp> display_arg
%type <str> data_type

%%

program:
    statement_list
    ;


statement_list:
    statement_list statement
    | /* empty */
    ;

statement:
    display_statement {
        if(hasError) YYABORT;
    }
    | declaration_statement {
        if(hasError) YYABORT;
    }
    | assignment_statement {
        if(hasError) YYABORT;
    }
    | error SEMI { 
        yyerrok; 
        hasError = 1;
        YYABORT;
    }
    ; 

display_statement:
    DISPLAY '(' display_arg ')' SEMI {
        if(hasError) {
            if($3.text != NULL) free($3.text);
            YYABORT;
        }
        if($3.text != NULL){
            printf("LINE %d: %s\n", yylineno, $3.text);
            free($3.text);
        }
    }
    ;

display_arg:
    STRING { 
        $$.text = strdup($1);
        $$.type = 1;
        free($1);
    }
    | CHARACTER { 
        char buffer[2];
        buffer[0] = $1;
        buffer[1] = '\0';
        $$.text = strdup(buffer);
        $$.type = 2;
    }
    | VARIABLE {
        vars *var = getVariable($1);
        if(!var){
            yyerror("Undefined variable '%s' on line %d", $1, yylineno);
            hasError = 1;
            $$.text = NULL;
            $$.type = 4;
        } else {
            if(var->data_type == 's'){
                $$.text = strdup(var->data.str_val);
                $$.type = 1;
            } else if(var->data_type == 'c'){
                char buffer[2];
                buffer[0] = (char)var->data.val;
                buffer[1] = '\0';
                $$.text = strdup(buffer);
                $$.type = 2;
            } else {
                char buffer[20];
                sprintf(buffer, "%d", var->data.val);
                $$.text = strdup(buffer);
                $$.type = 3;
            }
        }
        free($1);
    }
    | INTEGER {
        char buffer[20];
        sprintf(buffer, "%d", $1);
        $$.text = strdup(buffer);
        $$.type = 3;
    }
    | '(' expression ')' {
        char buffer[20];
        sprintf(buffer, "%d", $2);
        $$.text = strdup(buffer);
        $$.type = 3;
    }
    | display_arg '+' display_arg {
        if(hasError) {
            if($1.text) free($1.text);
            if($3.text) free($3.text);
            $$.text = NULL;
            $$.type = 4;
            YYERROR;
        }
        if($1.text == NULL || $3.text == NULL) {
            $$.text = NULL;
            $$.type = 4;
            if($1.text) free($1.text);
            if($3.text) free($3.text);
            YYERROR;
        } else if($1.type == 1 || $3.type == 1) {
            // String concatenation: if either operand is a string
            size_t len = strlen($1.text) + strlen($3.text) + 1;
            $$.text = malloc(len);
            if($$.text) {
                strcpy($$.text, $1.text);
                strcat($$.text, $3.text);
                $$.type = 1;  // Result is a string
            } else {
                yyerror("Memory allocation failed on line %d", yylineno);
                $$.text = NULL;
                $$.type = 4;
                hasError = 1;
            }
            free($1.text);
            free($3.text);
        } else {
            // Numeric addition
            int val1 = atoi($1.text);
            int val2 = atoi($3.text);
            char buffer[20];
            sprintf(buffer, "%d", val1 + val2);
            $$.text = strdup(buffer);
            $$.type = 3;
            free($1.text);
            free($3.text);
        }
    }
    | display_arg '-' display_arg {
        if(hasError) {
            if($1.text) free($1.text);
            if($3.text) free($3.text);
            $$.text = NULL;
            $$.type = 4;
            YYERROR;
        }
        if($1.text == NULL || $3.text == NULL) {
            $$.text = NULL;
            $$.type = 4;
            if($1.text) free($1.text);
            if($3.text) free($3.text);
            YYERROR;
        } else {
            int val1 = atoi($1.text);
            int val2 = atoi($3.text);
            char buffer[20];
            sprintf(buffer, "%d", val1 - val2);
            $$.text = strdup(buffer);
            $$.type = 3;
            free($1.text);
            free($3.text);
        }
    }
    | display_arg '*' display_arg {
        if(hasError) {
            if($1.text) free($1.text);
            if($3.text) free($3.text);
            $$.text = NULL;
            $$.type = 4;
            YYERROR;
        }
        if($1.text == NULL || $3.text == NULL) {
            $$.text = NULL;
            $$.type = 4;
            if($1.text) free($1.text);
            if($3.text) free($3.text);
            YYERROR;
        } else {
            int val1 = atoi($1.text);
            int val2 = atoi($3.text);
            char buffer[20];
            sprintf(buffer, "%d", val1 * val2);
            $$.text = strdup(buffer);
            $$.type = 3;
            free($1.text);
            free($3.text);
        }
    }
    | display_arg '/' display_arg {
        if(hasError) {
            if($1.text) free($1.text);
            if($3.text) free($3.text);
            $$.text = NULL;
            $$.type = 4;
            YYERROR;
        }
        if($1.text == NULL || $3.text == NULL) {
            $$.text = NULL;
            $$.type = 4;
            if($1.text) free($1.text);
            if($3.text) free($3.text);
            YYERROR;
        } else {
            int val1 = atoi($1.text);
            int val2 = atoi($3.text);
            if(val2 == 0){
                yyerror("Division by zero in display on line %d", yylineno);
                hasError = 1;
                $$.text = NULL;
                free($1.text);
                free($3.text);
                YYERROR;
            } else {
                char buffer[20];
                sprintf(buffer, "%d", val1 / val2);
                $$.text = strdup(buffer);
            }
            $$.type = 3;
            free($1.text);
            free($3.text);
        }
    }
    | '-' display_arg %prec UMINUS {
        if($2.text == NULL) {
            $$.text = NULL;
            $$.type = 4;
        } else {
            int val = atoi($2.text);
            char buffer[20];
            sprintf(buffer, "%d", -val);
            $$.text = strdup(buffer);
            $$.type = 3;
            free($2.text);
        }
    }
    | '+' display_arg %prec UMINUS {
        if($2.text == NULL) {
            $$.text = NULL;
            $$.type = 4;
        } else {
            $$ = $2;
        }
    }
    ;

declaration_statement:
    data_type declaration_list SEMI {
        if(currentDataType) {
            free(currentDataType);
            currentDataType = NULL;
        }
        if(currentVarBeingDeclared) {
            free(currentVarBeingDeclared);
            currentVarBeingDeclared = NULL;
        }
    }
    ;

assignment_statement:
    assignment_list SEMI
    ;

data_type:
    DATA_TYPE {
        currentDataType = strdup($1);
        $$ = currentDataType;
    }
    ;

declaration_list:
    var_decl 
    | declaration_list COMMA var_decl
    ;


var_decl:
    VARIABLE {
        // Initialize with default value: 0 for int/char, empty for string
        if(strcmp(currentDataType, "string") == 0) {
            createVariable(currentDataType, $1, 0, "");
        } else {
            createVariable(currentDataType, $1, 0, NULL);
        }
        if(hasError) YYABORT;
        free($1);
    }
    | VARIABLE ASSIGNMENT {
        // Set the variable being declared BEFORE evaluating expression
        currentVarBeingDeclared = strdup($1);
        
        // Check if we're trying to assign to string type with expression
        if(strcmp(currentDataType, "string") == 0) {
            yyerror("String expressions are not allowed. Cannot assign expression to string variable '%s' on line %d", 
                    currentVarBeingDeclared, yylineno);
            hasError = 1;
            free(currentVarBeingDeclared);
            currentVarBeingDeclared = NULL;
            free($1);
            YYABORT;
        }
    } expression {
        // Create variable with the expression value (only for non-string types)
        if(!hasError) {
            createVariable(currentDataType, $1, $4, NULL);
        }
        free(currentVarBeingDeclared);
        currentVarBeingDeclared = NULL;
        free($1);
        if(hasError) YYABORT;
    }
    | VARIABLE ASSIGNMENT STRING '+' {
        yyerror("String expressions are not allowed. Cannot use '+' operator with string variable '%s' on line %d", 
                $1, yylineno);
        hasError = 1;
        free($1);
        free($3);
        YYABORT;
    } STRING {
        // This action will never be reached due to YYABORT above
        free($6);
    }
    | VARIABLE ASSIGNMENT STRING {
        if(strcmp(currentDataType, "string") != 0) {
            yyerror("Cannot assign string value to %s variable '%s' on line %d", 
                    currentDataType, $1, yylineno);
            hasError = 1;
            free($1);
            free($3);
            YYABORT;
        } else {
            createVariable(currentDataType, $1, 0, $3);
            if(hasError) {
                free($1);
                free($3);
                YYABORT;
            }
        }
        free($1);
        free($3);
    }
    | VARIABLE ASSIGNMENT CHARACTER {
        if(strcmp(currentDataType, "string") == 0) {
            yyerror("Cannot assign character value to string variable '%s' on line %d", 
                    $1, yylineno);
            hasError = 1;
            free($1);
            YYABORT;
        } else {
            createVariable(currentDataType, $1, (int)$3, NULL);
            if(hasError) {
                free($1);
                YYABORT;
            }
        }
        free($1);
    }
    | VARIABLE ASSIGNMENT VARIABLE {
        // Handle string to string assignment (copy value from another string variable)
        if(strcmp(currentDataType, "string") == 0) {
            vars *sourceVar = getVariable($3);
            if(!sourceVar) {
                yyerror("Undefined variable '%s' on line %d", $3, yylineno);
                hasError = 1;
                free($1);
                free($3);
                YYABORT;
            } else if(sourceVar->data_type != 's') {
                yyerror("Cannot assign non-string variable to string variable '%s' on line %d", $1, yylineno);
                hasError = 1;
                free($1);
                free($3);
                YYABORT;
            } else {
                createVariable(currentDataType, $1, 0, sourceVar->data.str_val);
                if(hasError) {
                    free($1);
                    free($3);
                    YYABORT;
                }
            }
        } else {
            // For non-string types, treat as expression
            currentVarBeingDeclared = strdup($1);
            int val = getVariableValue($3);
            if(hasError) {
                free(currentVarBeingDeclared);
                currentVarBeingDeclared = NULL;
                free($1);
                free($3);
                YYABORT;
            }
            createVariable(currentDataType, $1, val, NULL);
            free(currentVarBeingDeclared);
            currentVarBeingDeclared = NULL;
            if(hasError) {
                free($1);
                free($3);
                YYABORT;
            }
        }
        free($1);
        free($3);
    }
    ;

assignment_list:
    assignment 
    | assignment_list COMMA assignment
    ;

assignment:
    VARIABLE ASSIGNMENT expression {
        vars *var = getVariable($1);
        if(var && var->data_type == 's') {
            yyerror("String expressions are not allowed. Cannot assign expression to string variable '%s' on line %d", $1, yylineno);
            hasError = 1;
        } else {
            variableReAssignment($1, $3, NULL);
        }
        free($1);
    }
    | VARIABLE ASSIGNMENT STRING '+' {
        vars *var = getVariable($1);
        if(!var) {
            yyerror("Undefined variable '%s' on line %d", $1, yylineno);
        } else {
            yyerror("String expressions are not allowed. Cannot use '+' operator with string variable '%s' on line %d", 
                    $1, yylineno);
        }
        hasError = 1;
        free($1);
        free($3);
        YYABORT;
    } STRING {
        // This action will never be reached due to YYABORT above
        free($6);
    }
    | VARIABLE ASSIGNMENT STRING {
        vars *var = getVariable($1);
        if(!var) {
            yyerror("Undefined variable '%s' on line %d", $1, yylineno);
            hasError = 1;
            free($1);
            free($3);
            YYABORT;
        } else if(var->data_type != 's') {
            yyerror("Cannot assign string value to non-string variable '%s' on line %d", $1, yylineno);
            hasError = 1;
            free($1);
            free($3);
            YYABORT;
        } else {
            variableReAssignment($1, 0, $3);
        }
        free($1);
        free($3);
        if(hasError) YYABORT;
    }
    | VARIABLE ASSIGNMENT CHARACTER {
        vars *var = getVariable($1);
        if(var && var->data_type == 's') {
            yyerror("Cannot assign character value to string variable '%s' on line %d", $1, yylineno);
            hasError = 1;
        } else {
            variableReAssignment($1, (int)$3, NULL);
        }
        free($1);
    }
    | VARIABLE ASSIGNMENT VARIABLE {
        vars *targetVar = getVariable($1);
        vars *sourceVar = getVariable($3);
        
        if(!targetVar) {
            yyerror("Undefined variable '%s' on line %d", $1, yylineno);
            hasError = 1;
        } else if(!sourceVar) {
            yyerror("Undefined variable '%s' on line %d", $3, yylineno);
            hasError = 1;
        } else if(targetVar->data_type == 's' && sourceVar->data_type == 's') {
            // String to string assignment is allowed
            variableReAssignment($1, 0, sourceVar->data.str_val);
        } else if(targetVar->data_type == 's' && sourceVar->data_type != 's') {
            yyerror("Cannot assign non-string variable to string variable '%s' on line %d", $1, yylineno);
            hasError = 1;
        } else if(targetVar->data_type != 's' && sourceVar->data_type == 's') {
            yyerror("Cannot assign string variable to non-string variable '%s' on line %d", $1, yylineno);
            hasError = 1;
        } else {
            // Non-string to non-string
            variableReAssignment($1, sourceVar->data.val, NULL);
        }
        free($1);
        free($3);
    }
    ;

expression:
    INTEGER { 
        $$ = $1; 
    }
    | CHARACTER { 
        $$ = (int)$1; 
    }
    | VARIABLE { 
        // Check if this variable is being declared right now
        if(currentVarBeingDeclared && strcmp($1, currentVarBeingDeclared) == 0) {
            yyerror("Variable '%s' used in its own initialization on line %d", 
                    $1, yylineno);
            hasError = 1;
            $$ = 0;
        } else {
            $$ = getVariableValue($1);
        }
        free($1);
    }
    | '(' expression ')' { 
        $$ = $2; 
    }
    | expression '+' expression { 
        $$ = $1 + $3; 
    }
    | expression '-' expression { 
        $$ = $1 - $3; 
    }
    | expression '*' expression { 
        $$ = $1 * $3; 
    }
    | expression '/' expression { 
        if($3 == 0){
            yyerror("Division by zero on line %d", yylineno);
            hasError = 1;
            $$ = 0;
        } else {
            $$ = $1 / $3;
        }
    }
    | '-' expression %prec UMINUS { 
        $$ = -$2; 
    }
    | '+' expression %prec UMINUS { 
        $$ = $2; 
    }
    ;

%%


int main(void) {
    printf("Welcome to my Custom sPyC!\n");
    int result = yyparse();
    if (headVars) printVariableTable();
    if (headErrList) printErrorTable();
    if (headErrList) cleanupErrorTable();
    if (headVars) cleanupVariableTable();
    return result;
}


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


/*--------------- Variable handling ----------------------------*/
int getVariableValue(char *variableName){
    if (isRecovering) return 0;
    
    vars *existing = getVariable(variableName);
    
    if(!existing){
        // Variable doesn't exist at all - this is an error
        yyerror("Undefined variable %s, on line %d.", variableName, yylineno);
        hasError = 1;
        return 0;
    }
    
    if(existing->data_type == 's'){
        yyerror("Cannot perform arithmetic operations on variable %s: string literals, on line %d.", variableName, yylineno);
        hasError = 1;
        return 0;
    }
    
    // Return the value (will be 0 if uninitialized, which is correct)
    return existing->data.val;
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