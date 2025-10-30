%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror(const char *s);
int yylex(void);

/* Symbol table structure */
typedef struct {
    char *name;
    char type;  /* 'i' for int, 'c' for char */
    union {
        int int_val;
        char char_val;
    } value;
} Symbol;

Symbol symbol_table[100];
int symbol_count = 0;
int has_error = 0;  /* Flag to track errors */

/* Helper functions */
int lookup_symbol(char *name);
int add_symbol(char *name, char type);  /* Returns 1 on success, 0 on error */
void set_symbol_value(char *name, int value);
int get_symbol_value(char *name);
%}

%union {
    int num;
    char c;
    char *str;
}

%token <num> NUMBER
%token <c> CHARACTER
%token <str> ID
%token INT CHAR
%token ADDITION SUBTRACTION MULTIPLICATION DIVISION
%token SEMICOLON COMMA ASSIGNMENT

%type <num> expression term factor

%left ADDITION SUBTRACTION
%left MULTIPLICATION DIVISION

%%

program:
    /* empty */
    | program statement
    ;

statement:
    declaration SEMICOLON       { 
                                    if (!has_error) {
                                        printf("Declaration processed\n"); 
                                    }
                                    has_error = 0;
                                }
    | assignment SEMICOLON      { 
                                    if (!has_error) {
                                        printf("Assignment processed\n"); 
                                    }
                                    has_error = 0;
                                }
    | expression SEMICOLON      { 
                                    if (!has_error) {
                                        printf("Expression result: %d\n", $1); 
                                    }
                                    has_error = 0;
                                }
    ;

declaration:
    INT int_declaration_list
    | CHAR char_declaration_list
    ;

int_declaration_list:
    ID                          { 
                                    if (add_symbol($1, 'i')) {
                                        printf("Declared variable: %s\n", $1);
                                    }
                                    free($1);
                                }
    | ID ASSIGNMENT expression  { 
                                    if (add_symbol($1, 'i')) {
                                        set_symbol_value($1, $3);
                                        printf("Declared and initialized %s = %d\n", $1, $3);
                                    }
                                    free($1);
                                }
    | int_declaration_list COMMA ID { 
                                    if (add_symbol($3, 'i')) {
                                        printf("Declared variable: %s\n", $3);
                                    }
                                    free($3);
                                }
    | int_declaration_list COMMA ID ASSIGNMENT expression {
                                    if (add_symbol($3, 'i')) {
                                        set_symbol_value($3, $5);
                                        printf("Declared and initialized %s = %d\n", $3, $5);
                                    }
                                    free($3);
                                }
    ;

char_declaration_list:
    ID                          { 
                                    if (add_symbol($1, 'c')) {
                                        printf("Declared char variable: %s\n", $1);
                                    }
                                    free($1);
                                }
    | ID ASSIGNMENT CHARACTER   { 
                                    if (add_symbol($1, 'c')) {
                                        symbol_table[symbol_count-1].value.char_val = $3;
                                        printf("Declared and initialized %s = '%c'\n", $1, $3);
                                    }
                                    free($1);
                                }
    | char_declaration_list COMMA ID { 
                                    if (add_symbol($3, 'c')) {
                                        printf("Declared char variable: %s\n", $3);
                                    }
                                    free($3);
                                }
    | char_declaration_list COMMA ID ASSIGNMENT CHARACTER {
                                    if (add_symbol($3, 'c')) {
                                        symbol_table[symbol_count-1].value.char_val = $5;
                                        printf("Declared and initialized %s = '%c'\n", $3, $5);
                                    }
                                    free($3);
                                }
    ;

assignment:
    ID ASSIGNMENT expression    { 
                                    int idx = lookup_symbol($1);
                                    if (idx >= 0) {
                                        set_symbol_value($1, $3);
                                        printf("Assigned %s = %d\n", $1, $3);
                                    } else {
                                        fprintf(stderr, "Error: Undeclared variable '%s'\n", $1);
                                        has_error = 1;
                                    }
                                    free($1);
                                }
    | ID ASSIGNMENT CHARACTER   { 
                                    int idx = lookup_symbol($1);
                                    if (idx >= 0) {
                                        if (symbol_table[idx].type == 'c') {
                                            symbol_table[idx].value.char_val = $3;
                                            printf("Assigned %s = '%c'\n", $1, $3);
                                        } else {
                                            fprintf(stderr, "Error: Type mismatch for '%s'\n", $1);
                                            has_error = 1;
                                        }
                                    } else {
                                        fprintf(stderr, "Error: Undeclared variable '%s'\n", $1);
                                        has_error = 1;
                                    }
                                    free($1);
                                }
    ;

expression:
    term                        { $$ = $1; }
    | expression ADDITION term  { $$ = $1 + $3; }
    | expression SUBTRACTION term { $$ = $1 - $3; }
    ;

term:
    factor                      { $$ = $1; }
    | term MULTIPLICATION factor { $$ = $1 * $3; }
    | term DIVISION factor      { 
                                    if ($3 == 0) {
                                        yyerror("Division by zero");
                                        has_error = 1;
                                        $$ = 0;
                                    } else {
                                        $$ = $1 / $3;
                                    }
                                }
    ;

factor:
    NUMBER                      { $$ = $1; }
    | ID                        { 
                                    int idx = lookup_symbol($1);
                                    if (idx >= 0) {
                                        if (symbol_table[idx].type == 'i') {
                                            $$ = symbol_table[idx].value.int_val;
                                        } else {
                                            $$ = (int)symbol_table[idx].value.char_val;
                                        }
                                    } else {
                                        fprintf(stderr, "Error: Undeclared variable '%s'\n", $1);
                                        has_error = 1;
                                        $$ = 0;
                                    }
                                    free($1);
                                }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int lookup_symbol(char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int add_symbol(char *name, char type) {
    if (lookup_symbol(name) >= 0) {
        fprintf(stderr, "Error: Variable '%s' is already declared\n", name);
        has_error = 1;
        return 0;  /* Return failure */
    }
    if (symbol_count < 100) {
        symbol_table[symbol_count].name = strdup(name);
        symbol_table[symbol_count].type = type;
        symbol_table[symbol_count].value.int_val = 0;
        symbol_count++;
        return 1;  /* Return success */
    }
    fprintf(stderr, "Error: Symbol table full\n");
    has_error = 1;
    return 0;
}

void set_symbol_value(char *name, int value) {
    int idx = lookup_symbol(name);
    if (idx >= 0) {
        symbol_table[idx].value.int_val = value;
    }
}

int get_symbol_value(char *name) {
    int idx = lookup_symbol(name);
    if (idx >= 0) {
        return symbol_table[idx].value.int_val;
    }
    return 0;
}

int main(void) {
    printf("Enter your code (Ctrl+D to end):\n");
    return yyparse();
}