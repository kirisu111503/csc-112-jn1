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
void getVariableValue(char *variableName);
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

prog
%%


program:
    statement_list
    ;


statement_list:
    statement statement_list |
    ;

statement:
    display_statment | 
    declaration_statement | 
    assignment_statement | 
    expression_statement 
    ; 

display_statement:
    DISPLAY "(" display_arg ")" ";"
    ;

display_arg:
    string_literal |
    char_literal |
    identifier |
    expression 
    ;

declaration_statement:
    data_type declaration_list ";"
    ;

assignment_statement:
    assignment_list
    ;

expression_statement:
    expression
    ;



declaration_list:
    var_decl |
    declaration_list "," var_decl
    ;


var_decl:
    identifier |
    declaration_list "," expression |
    identifier "=" string_literal |
    identifier "=" char_literal
    ;

assignment_list:
    assignment |
    assignment_list "," assignment
    ;

assignment:
    identifier "=" expression |
    identifier "=" string_literal |
    identifier "=" char_literal
    ;

expression:
    term |
    expression "+" term |
    expression "-" term 
    ;

term:
    factor | term "+" factor | term "/" factor 
    ;

factor:
    "-" factor 
    ;

primary:
    Number | char_literal | identifier | "(" expression ")" 
    ;

number:
    digit number_tail
    ;

number_tail:
    digit number_tail | 
    ;

digit:
    DIGIT
    ;


data_type:
    int | char | string
    ;

identifier:
    letter id_tail 
    ;

id_tail:
    letter id_tail |
    digit id_tail |
    "_" id_tail |
    ;

letter:
    CHARACTER
    ;

char_literal:
    "'"character"'"
    ;

character:
    letter | digit | special_char 
    ;

string_literal:
    "'"string_body"'"
    ;
string_body:
    string_char string_body | 
    ;

string_char:
    letter | digit | special_char | " "
    ;

special_char:
    "!" | "@" | "#" | "$" | "%" | "^" | "&" | "*"| "(" | ")" | "-" | "_" | "+" | "=" | "{" | "}"| "[" | "]" | "|" | "" | ":" | ";" | "<" | ">"| "," | "." | "?" | "/" | "~" | "`" | " "
    ;

%%


int main(void) {
    printf("Welcome to my Custom sPyC!\n");
    int result = yyparse();
    if (headVars) printVariableTable(); // Display all Variables.
    if (headErrList) printErrorTable(); // Display all erros if exist.
    if (headErrList) cleanupErrorTable(); // Free memories alocated on error list.
    if (headVars) cleanupVariableTable();  // Free memories of all variable list.
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
int getVariableValue(char *variableName){
    if (isRecovering) return 0;
    
    vars *existing = getVariable(variableName);
    
    if(!existing){
        yyerror("Undefined variable %s, on line %d.", variableName, yylineno);
        return 0;  // Return default on error
    }
    
    if(existing->data_type == 's'){
        yyerror("Cannot perform arithmetic operations on variable %s: string literals, on line %d.", variableName, yylineno);
        return 0;
    }
    
    return existing->data.val;  // return the value
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

void createVariable(char *DTYPE, char* variable, int val, char *str_val) {  
    // Validate variable name first
    if(isdigit(variable[0])){
        yyerror("Variable %s can't start in number, in line %d.", variable, yylineno);
        return;
    }

    vars *existing = getVariable(variable);

    // Declaration with type keyword when variable exists
    if (existing && DTYPE && strlen(DTYPE) > 0) {
        yyerror("Variable '%s' is already declared with type '%s' on line %d", 
                variable, typeName(existing->data_type), yylineno);
        return;
    }

    // Reassignment case (variable exists, no type keyword)
    if (existing && (!DTYPE || strlen(DTYPE) == 0)) {
        variableReAssignment(variable, val, str_val);
        return;
    }

    // Assignment without type keyword when variable doesn't exist
    if (!existing && (!DTYPE || strlen(DTYPE) == 0)) {
        yyerror("Undefined variable '%s' on line %d", variable, yylineno);
        return;
    }

    // DTYPE is now guaranteed to be non-NULL and non-empty

    // Type/value mismatch checks
    if (strcmp(DTYPE, "int") == 0 && str_val != NULL) {
        yyerror("Cannot assign string value to int variable '%s' on line %d", variable, yylineno);
        return;
    }
    if (strcmp(DTYPE, "char") == 0 && str_val != NULL) {
        yyerror("Cannot assign string value to char variable '%s' on line %d", variable, yylineno);
        return;
    }
    if (strcmp(DTYPE, "string") == 0 && str_val == NULL) {
        yyerror("Cannot assign non-string value to string variable '%s' on line %d", variable, yylineno);
        return;
    }

    // Create the variable
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
        newVar->data.str_val = str_val ? strdup(str_val) : strdup("");
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
        return;
    }

    // Type checking
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

    // Assign value
    if(existing->data_type == 'i' || existing->data_type == 'c'){
        existing->data.val = val;
    } else {
        // Allocate new string first (safer approach)
        char *new_str = strdup(str_val ? str_val : "");
        if(!new_str){
            yyerror("Failed to allocate memory for str value on line %d", yylineno);
            return;  // Old value preserved
        }
        // Only free old string after successful allocation
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