%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

extern int lineCount;
%}

%option yylineno

%%
"display"                               { return DISPLAY; }
"int"                                   { yylval.str = strdup("int"); return DATA_TYPE; }
"char"                                  { yylval.str = strdup("char"); return DATA_TYPE; }
"string"                                { yylval.str = strdup("string"); return DATA_TYPE; }

[a-zA-Z_][a-zA-Z0-9_]*                  { yylval.str = strdup(yytext); return VARIABLE; }
"="                                     { return ASSIGNMENT; }
\"[^\"]*\"                              { 
                                          int len = strlen(yytext) - 2;
                                          yylval.str = (char*)malloc(len + 1);
                                          strncpy(yylval.str, yytext + 1, len);
                                          yylval.str[len] = '\0';
                                          return STRING; 
                                        }
[0-9]+                                  { yylval.num = atoi(yytext); return INTEGER; }
'.'                                     { yylval.character = yytext[1]; return CHARACTER; }
[0-9]                                   { yylval.num = atoi(yytext); return DIGIT; }
[-+*/()]                                { return yytext[0]; }
","                                     { return COMMA; }
";"                                     { return SEMI; }

[ \t\r\n]+                              { /* ignore whitespace */ }

.                                       { fprintf(stderr, "Invalid character: %s\n", yytext); }

%%

int yywrap(void) { return 1; }