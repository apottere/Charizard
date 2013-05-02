%{
#include "y.tab.h"
#include "tree.h"
#include "vector.h"
%}


%%
not						return NOT;
while					return WHILE;
do						return DO;
if						return IF;
then					return THEN;
else					return ELSE;
begin					return BEGIN_T;
end						return END;
program					return PROGRAM;
function				return FUNCTION;
of						return OF;
array					return ARRAY;
var						return VAR;
integer					return INTEGER;
procedure				return PROCEDURE;
[=|<>|<|<=|>|>=]		yylval=make_tree(RELOP, NULL); yylval->attribute.name = strdup(yytext); return RELOP;


[(]						return OBRACK;
[)]						return EBRACK;
\[						return OSBRACK;
\]						return ESBRACK;

or						return OR;
[*|/]|div|mod|and		yylval=make_tree(MULOP, NULL); yylval->attribute.name = strdup(yytext); return MULOP;
[+|-]					yylval=make_tree(SIGN, NULL); yylval->attribute.name = strdup(yytext); return SIGN;

,						return COMMA;
\.						return DOT;
:						return COLON;
\;						return SEMICOLON;
:=						return ASSIGNOP;

\n                      
[ \t]+

[0-9]+					yylval=make_tree(NUM, NULL); yylval->attribute.ival = atoi(yytext); return NUM;
[a-zA-Z][a-zA-Z0-9]*    yylval=make_tree(IDENT, NULL); yylval->attribute.name = strdup(yytext); return IDENT;
%%
