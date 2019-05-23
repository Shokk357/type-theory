%{
#include "../expression.h"
#include <iostream>
int yylex(void);
void yyerror(const char *);
expression* result = nullptr;
%}

%union {
    expression* e;
    std::string* id;
}
%token LET
%token IN
%token EQ
%token <id> ID
%token POINT
%token BACK
%token LEFT RIGHT

%type <e> Input Expr Abst Prim Term Per

%start Input

%%
Input: Expr{ result = $1; }
;

Expr:
	LET ID EQ Expr IN Expr { $$ = new let($2, $4, $6); }
	| Abst { $$ = $1; }
;

Abst:
	Prim BACK ID POINT Abst { $$ = new application($1, new abstraction($3, $5)); }
	| BACK ID POINT Abst { $$ = new abstraction($2, $4);}
	| Prim { $$ = $1; }
;

Prim:
	Prim Term { $$ = new application($1, $2);}
	| Term { $$ = $1;}
;

Term:
	LEFT Expr RIGHT { $$ = $2;}
	| Per { $$ = $1;}
;

Per:
	ID { $$ = new variable($1); }
;
%%

