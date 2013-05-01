%{
#include "y.tab.h"
#include "tree.h"
%}

digit			[0-9]
sign			[+|-]
addop			{sign}|or


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
[=|<>|<|<=|>|>=]		return RELOP;


[(]						return OBRACK;
[)]						return EBRACK;
\[						return OSBRACK;
\]						return ESBRACK;

{addop}					return ADDOP;
[*|/]|div|mod|and		return MULOP;
{sign}					return SIGN;

,						return COMMA;
\.						return DOT;
:						return COLON;
\;						return SEMICOLON;
:=						return ASSIGNOP;

\n                      
[ \t]+

{digit}+					yylval=make_tree(NUM, NULL, NULL); yylval->attribute.ival = atoi(yytext); return NUM;
[a-zA-Z][a-zA-Z{digit}]*    yylval=make_tree(IDENT, NULL, NULL); yylval->attribute.name = strdup(yytext); return IDENT;
%%
