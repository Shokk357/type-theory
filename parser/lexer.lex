%{
#include "../expression.h"
#include "parser.h"
%}

%%
"let" return LET;
"in"  return IN;
"="   return EQ;
([a-z][a-z0-9]*) {
    yylval.id = new string(yytext);
    return ID;
}
"\\" return BACK;
"\." return POINT;
"(" return LEFT;
")" return RIGHT;
([ \t\n\r])
%%