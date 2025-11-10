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


//Initialization of structure
//Variables
// extern vars *headVars;
// extern vars *tailVars ;

//errors
// extern errorList *headErrList;
// extern errorList *tailErrList;

//logs
// extern logs *headLogs;
// extern logs *tailLogs;

// extern char *currentDataType;

extern int yylineno;

vars *headVars = NULL;
vars *tailVars = NULL;
errorList *headErrList = NULL;
errorList *tailErrList = NULL;
logs *headLogs = NULL;
logs *tailLogs = NULL;
char *currentDataType = NULL;
int isRecovering = 0;

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



%union {
    int num;
    char character;
    char *str;
}

%token <str> DATA_TYPE
%token <str> VARIABLE
%token ASSIGNMENT
%token DISPLAY
%token COMMA
%token SEMI
%token <str> STRING 
%token <num> INTEGER
%token <character> CHARACTER

%left COMMA
%left '+' '-'
%left '*' '/'
%right ASSIGNMENT


//All functions here

%type <num> expr term factor


%%

program:
    statement_list
    ;

statement_list:
    statement_list statement
  | statement
  ;

statement:
    DISPLAY '(' STRING ')' SEMI { 
        printf("LINE %d: %s\n",yylineno , $3); 
        free($3); 
    }
   | DISPLAY '(' VARIABLE ')' SEMI {
        vars *var = getVariable($3);
        if (var) {
            if (var->data_type == 's') {
                printf("LINE %d: %s\n", yylineno, var->data.str_val);
            } else {
                printf("LINE %d: %d\n",yylineno , var->data.val);
            }
        }
        free($3);
    }
  | DISPLAY '(' expr ')' SEMI { 
        printf("LINE %d: %d\n", yylineno, $3);
    }
  | DATA_TYPE {currentDataType = $1; } declaration_list SEMI {
        currentDataType = NULL;
    }
  | assignment_list SEMI
  | expr SEMI
  | error SEMI {yyerrok; }
  ;

declaration_list:
    var_decl
  | declaration_list COMMA var_decl
  ;

assignment_list:
    assignment
  | assignment_list COMMA assignment
  ;

var_decl:
    VARIABLE {
        if(strcmp(currentDataType, "string")==0){
            createVariable(currentDataType, $1, 0, "");
        }else{
            createVariable(currentDataType, $1, 0, NULL);
        }
        free($1);
    }
  | VARIABLE ASSIGNMENT expr {
        createVariable(currentDataType, $1, $3, NULL);
        free($1);
    }
  | VARIABLE ASSIGNMENT STRING{
        createVariable(currentDataType, $1, 0, $3);
        free($1);
        free($3);
    }
  | VARIABLE ASSIGNMENT CHARACTER { 
        createVariable(currentDataType, $1, (int)$3, NULL);
        free($1);
    }
  ;

assignment:
    VARIABLE ASSIGNMENT expr {
        variableReAssignment($1, $3, NULL);
        free($1);
    }
  | VARIABLE ASSIGNMENT STRING {
        variableReAssignment($1, 0, $3);
        free($1);
        free($3);
    }
  | VARIABLE ASSIGNMENT CHARACTER {
        variableReAssignment($1, (int)$3, NULL);
        free($1);
    }
  ;

expr:
    expr '+' term   {$$ = $1 + $3;}
  | expr '-' term   {$$ = $1 - $3;}
  | term            {$$ = $1;}
  ;

term:
    term '*' factor {$$ = $1 * $3;}
  | term '/' factor {
        if($3 == 0){
            yyerror("Division by zero, on line %d.", yylineno);
            $$ = 0;
        }else{
            $$ = $1 / $3;
        }
    }
  | factor {$$ = $1;}
  ;

factor:
    '(' expr ')'    {$$ = $2;}
  | INTEGER         {$$ = $1;}
  | CHARACTER       {$$ = (int)$1;}
  | VARIABLE        {
            $$ = getVariableValue($1); 
            free($1);
        }
  ;

%%


int main(void) {
    printf("Welcome to my Custom Zypher!\n");
    int result = yyparse();
    if (headVars) printVariableTable();
    if (headErrList) printErrorTable();
    if (headErrList) cleanupErrorTable();
    if (headVars) cleanupVariableTable();
    return result;
}


/*---------------------------------Error handling---------------------------------------------------*/
void yyerror(const char *fmt, ...) {
    // Filter out Bison's default "syntax error"
    if (fmt && strcmp(fmt, "syntax error") == 0)
        return;

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
//Free all memory for variable table

int getVariableValue(char *variableName){
    if (isRecovering) return 0;

    vars *existing = getVariable(variableName);

    if(!existing){
        yyerror("Undefined variable %s, on line %d.", variableName, yylineno);
        return 0;
    }

    if(existing->data_type == 's'){
        yyerror("Cannot perform arithmetic operations on variable %s: string literals, on line %d.", variableName, yylineno);
        return 0;
    }

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

// get the type name for error handling in variable
const char* typeName(char dt) {
    return (dt=='i')?"int":(dt=='c')?"char":"string";
}


// Returns the pointer to the variable if found, NULL otherwise
vars* getVariable(char* variable) {
    vars *current = headVars;
    while (current) {
        if (strcmp(current->id, variable) == 0) {
            return current; // found
        }
        current = current->next;
    }
    return NULL; // not found
}

// Check and Create Variable
void createVariable(char *DTYPE, char* variable, int val, char *str_val) {  
    // For declaration and assignment
    vars *existing = getVariable(variable);

    if(isdigit(variable[0])){
        yyerror("Variable %s can't start in number, in line %d.", variable, yylineno);
        return;
    }

    // Declaration with type keyword
    if (existing && DTYPE && strlen(DTYPE) > 0) {
        yyerror("Variable '%s' is already declared with type '%s' on line %d", 
                variable, typeName(existing->data_type), yylineno);
        return;
    }

    // Assignment without type keyword
    if (!existing && (!DTYPE || strlen(DTYPE) == 0)) {
        yyerror("Undefined variable '%s' on line %d", variable, yylineno);
        return;
    }

    // Type/value mismatch check
    if (DTYPE && strcmp(DTYPE, "int") == 0 && str_val != NULL) {
        yyerror("Cannot assign string value to int variable '%s' on line %d", variable, yylineno);
        return;
    }
    if (DTYPE && strcmp(DTYPE, "char") == 0 && str_val != NULL) {
        yyerror("Cannot assign string value to char variable '%s' on line %d", variable, yylineno);
        return;
    }
    if (DTYPE && strcmp(DTYPE, "string") == 0 && str_val == NULL) {
        yyerror("Cannot assign non-string value to string variable '%s' on line %d", variable, yylineno);
        return;
    }


    // no further error then proceed to create the variable
    vars *newVar = calloc(1, sizeof(vars));
    if (!newVar) {
        printf("Failed to allocate memory for variable '%s'\n", variable);
        return;
    }

    if (strcmp("int", DTYPE) == 0) {
        newVar->data_type = 'i';
        newVar->data.val = val;
    } else if (strcmp("char", DTYPE) == 0) {
        // NOTE: char stored as int (ASCII value)
        newVar->data_type = 'c';
        newVar->data.val = val;
    } else if (strcmp("string", DTYPE) == 0) {
        newVar->data_type = 's';
        newVar->data.str_val = str_val ? strdup(str_val) : strdup("");
    }
    else {
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
//NOTE: createVariable both declaration and assignment, require a data type
//NOTE: variableAssignment only re-assignment that doesn't require data type

void variableReAssignment(char* variable, int val, char *str_val){
    vars *existing = getVariable(variable);

    if(!existing){
        yyerror("Undefined variable %s on line %d.", variable, yylineno);
        return;
    }

    if (existing->data_type == 'i' && str_val != NULL) {
        yyerror("Cannot assign string value to int variable '%s' on line %d", variable, yylineno);
        return;
    }
    if (existing->data_type == 'c' && str_val != NULL) {
        yyerror("Cannot assign string value to char variable '%s' on line %d", variable, yylineno);
        return;
    }
    if (existing->data_type == 's' && str_val == NULL) {
        yyerror("Cannot assign non-string value to string variable '%s' on line %d", variable, yylineno);
        return;
    }

    //Assign value IDK
    if(existing->data_type == 'i' || existing->data_type == 'c'){
        existing->data.val = val;
    }else{
        free(existing->data.str_val);
        existing->data.str_val = strdup(str_val ? str_val : "");
        if(!existing->data.str_val){
            yyerror("Failed to allocate memory for str value on line %d", yylineno);
            return;
        }
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