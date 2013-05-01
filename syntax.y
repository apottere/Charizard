%code requires
{
#include "tree.h"
}

%{
#include <stdio.h>
#include <string.h>
#define YYSTYPE tree_t*
 
void yyerror(const char *str)
{
        fprintf(stderr,"error: %s\n",str);
}
 
int yywrap()
{
        return 1;
} 
  
main()
{
        yyparse();
} 

%}

%token IDENT NUM INTEGER
%token OBRACK EBRACK OSBRACK ESBRACK
%token ADDOP MULOP SIGN RELOP
%token NOT ASSIGNOP
%token WHILE DO IF THEN ELSE BEGIN_T END
%token COLON SEMICOLON COMMA DOT
%token PROGRAM FUNCTION RESERVED ARRAY OF VAR PROCEDURE

%%

program:
	   PROGRAM IDENT OBRACK identifier_list EBRACK SEMICOLON declarations subprogram_declarations compound_statement DOT

identifier_list:
			   IDENT
			   |
			   identifier_list COMMA IDENT
			   ;

declarations:
			declarations VAR identifier_list COLON type SEMICOLON
			|
			;

type:
	standard_type
	|
	ARRAY OSBRACK NUM DOT DOT NUM ESBRACK OF standard_type

standard_type:
			 INTEGER

subprogram_declarations:
					   subprogram_declarations subprogram_declaration SEMICOLON
					   |
					   ;

subprogram_declaration:
					  subprogram_head declarations compound_statement

subprogram_head:
			   FUNCTION IDENT arguments COLON standard_type SEMICOLON
			   |
			   PROCEDURE IDENT arguments SEMICOLON
			   ;

arguments:
		 OBRACK parameter_list EBRACK
		 |
		 ;

parameter_list:
			  identifier_list COLON type
			  |
			  parameter_list SEMICOLON identifier_list COLON type
			  ;


compound_statement:
				  BEGIN_T optional_statements END

optional_statements:
				   statement_list
				   |
				   ;

statement_list:
			  statement
			  |
			  statement_list SEMICOLON statement
			  ;

statement:
		 variable ASSIGNOP expr
		 |
		 procedure_statement
		 |
		 compound_statement
		 |
		 IF expr THEN statement ELSE statement
		 |
		 WHILE expr DO statement

variable:
		IDENT
		|
		IDENT OSBRACK expr ESBRACK
		;

procedure_statement:
				   IDENT 
				   |
				   IDENT OBRACK expr_list EBRACK
				   ;

expr_list:
		 expr
		 |
		 expr_list COMMA expr
		 {
	//		fprintf( stderr, "DOLLAR DOLLAR:\n");
	//		print_tree($$, 0);
	//		fprintf( stderr, "\n");
	//		fprintf( stderr, "-------------------------\n");
		 }
		 ;

expr:
	 simple_expr
	 |
	 simple_expr RELOP simple_expr
	 {
	 }
	 ;

simple_expr:
	term
	|
	sign term
	|
	simple_expr ADDOP term
	{/*
		if($2 == NULL) {
			$$ = $1;
		} else {
			$2->left = $1;
			$$ = $2;
		} */
	}
	;

term:
	 factor
	  {
	  }
	 |
	 term MULOP factor
	 {
	 }
	 ;

factor:
	  IDENT
	  |
	  IDENT OBRACK expr_list EBRACK
	  {
//	  	$$  = $2;
	  }
	  |
	  NUM
	  {
//	  	printf("\t->Found a number: %d\n", $1->attribute.ival);
//		$$ = $1;
	  }
	  |
	  OBRACK expr EBRACK
	  {
//	  	printf("\t->Found an identifier: %s\n", $1->attribute.name);
//		$$ = $1;
	  }
	  |
	  NOT factor
	  ;

sign:
	SIGN

%%
