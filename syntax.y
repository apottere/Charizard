%code requires
{
#include "tree.h"
#include "vector.h"
}

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define YYSTYPE tree_t*
 
void yyerror(const char *str)
{
		char* prefix = "cowsay -d error: ";
		char* error = (char*) malloc(sizeof(char)*(strlen(prefix) + strlen(str)));
		sprintf(error, "cowsay -d error: %s\n",str);
        system(error);
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
%token MULOP SIGN RELOP
%token NOT OR ASSIGNOP
%token WHILE DO IF THEN ELSE BEGIN_T END
%token COLON SEMICOLON COMMA DOT
%token PROGRAM FUNCTION ARRAY OF VAR PROCEDURE

// Too lazy to make my own enumeration!
%token RESERVED STATEMENT_LIST FUNCTION_CALL EXPRESSION_LIST UNARY_SIGN DECLARATION DECLARATION_LIST IDENTIFIER_LIST ARRAY_TYPE FUNCTION FUNCTION_LIST TYPE PARAMETER PARAMETER_LIST FUNCTION_HEADER PROCEDURE_HEADER

%%

program:
	   PROGRAM IDENT OBRACK identifier_list EBRACK SEMICOLON declarations subprogram_declarations compound_statement DOT
	   {
	   	fprintf(stderr, "Program found:\n");
		vector* children = vector_malloc();
		vector_add(children, $7);
		vector_add(children, $8);
		vector_add(children, $9);
		tree_t* final = make_tree(PROGRAM, children);
		print_tree(final, 0);

		system("cowsay -f dragon Charizard approves of your program.");
	   }
	   ;


identifier_list:
			   IDENT
			   {
				vector* children = vector_malloc();
				vector_add(children, $1);
				$$ = make_tree(IDENTIFIER_LIST, children);
			   }
			   |
			   identifier_list COMMA IDENT
			   {
			   	vector_add($1->children, $3);
			   }
			   ;

declarations:
			declarations VAR identifier_list COLON type SEMICOLON
			{
				if($1 == NULL) {
					vector* children = vector_malloc();
					vector_add(children, $3);
					vector_add(children, $5);

					vector* children2 = vector_malloc();
					vector_add(children2, make_tree(DECLARATION, children));
					$$ = make_tree(DECLARATION_LIST, children2);

				} else {
					vector* children = vector_malloc();
					vector_add(children, $3);
					vector_add(children, $5);

					vector_add($1->children, make_tree(DECLARATION, children));
					$$ = $1;
				}
			}
			|
			{
				$$ = NULL;
			}
			;

type:
	standard_type
	{
	}
	|
	ARRAY OSBRACK NUM DOT DOT NUM ESBRACK OF standard_type
	{
		vector* children = vector_malloc();
		vector_add(children, $3);
		vector_add(children, $6);
		

		vector* children2 = vector_malloc();
		vector_add(children2, make_tree(ARRAY_TYPE, children));
		$$ = make_tree(TYPE, children2);
	}
	;

standard_type:
			 INTEGER
			 {
				vector* children = vector_malloc();
				vector_add(children, $1);
				$$ = make_tree(TYPE, children);
			 	$$ = make_tree(INTEGER, NULL);
			 }
			 ;

subprogram_declarations:
					   subprogram_declarations subprogram_declaration SEMICOLON
					   {
					   	if($1 == NULL) {
							vector* children = vector_malloc();
							vector_add(children, $2);
							$$ = make_tree(FUNCTION_LIST, children);

						} else {
							vector_add($1->children, $2);
							$$ = $1;
						}
					   }
					   |
					   {
							$$ = NULL;
					   }
					   ;

subprogram_declaration:
					  subprogram_head declarations compound_statement
					  {
						vector* children = vector_malloc();
						vector_add(children, $1);
						vector_add(children, $2);
						vector_add(children, $3);
						$$ = make_tree(FUNCTION, children);
					  	
					  }
					  ;

subprogram_head:
			   FUNCTION IDENT arguments COLON standard_type SEMICOLON
			   {
					vector* children = vector_malloc();
					vector_add(children, $2);
					vector_add(children, $3);
					vector_add(children, $5);
					$$ = make_tree(FUNCTION_HEADER, children);
			   }
			   |
			   PROCEDURE IDENT arguments SEMICOLON
			   {
					vector* children = vector_malloc();
					vector_add(children, $2);
					vector_add(children, $3);
					$$ = make_tree(PROCEDURE_HEADER, children);
			   }
			   ;

arguments:
		 OBRACK parameter_list EBRACK
		 {
		 	$$ = $2;
		 }
		 |	
		 {
		 	$$ = NULL;
		 }
		 ;

parameter_list:
			  identifier_list COLON type
			  {
					vector* children = vector_malloc();
					vector_add(children, $1);
					vector_add(children, $3);

					vector* children2 = vector_malloc();
					vector_add(children2, make_tree(PARAMETER, children));
					$$ = make_tree(PARAMETER_LIST, children2);
			  	
			  }
			  |
			  parameter_list SEMICOLON identifier_list COLON type
			  {
			  	
					vector* children = vector_malloc();
					vector_add(children, $3);
					vector_add(children, $5);
					vector_add($1->children, make_tree(PARAMETER, children));
					$$ = $1;
			  }
			  ;


compound_statement:
				  BEGIN_T optional_statements END
				  {
				  	$$ = $2;
				  }

optional_statements:
				   statement_list
				   {
				   	$$ = $1;
				   }
				   |
				   {
				   	$$ = NULL;
				   }
				   ;

statement_list:
			  statement SEMICOLON
			  {
				fprintf(stderr, "TERM-----------------------------------------------\n");
				fprintf(stderr, "DOLLAR 1:\n");
				print_tree($1, 0);

			  	vector* children = vector_malloc();
				vector_add(children, $1);
			  	$$ = make_tree(STATEMENT_LIST, children);

				fprintf(stderr, "DOLLAR DOLLAR:\n");
				print_tree($$, 0);
				fprintf(stderr, "-------------------------------------------------\n");
			  }
			  |
			  statement_list statement SEMICOLON
			  {
			  	vector_add($1->children, $2);
				$$ = $1;
			  }
			  ;

statement:
		 variable ASSIGNOP expr
		 {
			fprintf(stderr, "STATEMENT: ASSIGN-----------------------------------------------\n");
			fprintf(stderr, "DOLLAR 1:\n");
			print_tree($1, 0);
			fprintf(stderr, "DOLLAR 3:\n");
			print_tree($3, 0);

		 	fprintf(stderr, "FOUND ASSIGN OP\n");
			vector* children = vector_malloc();
			vector_add(children, $1);
			vector_add(children, $3);
		 	$$ = make_tree(ASSIGNOP, children);

			fprintf(stderr, "DOLLAR DOLLAR:\n");
			print_tree($$, 0);
			fprintf(stderr, "-------------------------------------------------\n");
		 }
		 |
		 procedure_statement
		 |
		 {
		 	fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!UNWRITTEN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		 }
		 compound_statement
		 {
		 	fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!UNWRITTEN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		 }
		 |
		 IF expr THEN statement ELSE statement
		 {
		 	fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!UNWRITTEN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		 }
		 |
		 WHILE expr DO statement
		 {
		 	fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!UNWRITTEN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		 }
		 ;

variable:
		IDENT
		{
			fprintf(stderr, "VARIABLE-----------------------------------------------\n");
			fprintf(stderr, "DOLLAR 1:\n");
			print_tree($1, 0);

			$$ = $1;

			fprintf(stderr, "DOLLAR DOLLAR:\n");
			print_tree($$, 0);
			fprintf(stderr, "-------------------------------------------------\n");
		}
		|
		IDENT OSBRACK simple_expr ESBRACK
		{
			printf("\t->Found an array: %s\n", $1->attribute.name);
			vector* children = vector_malloc();
			vector_add(children, $1);
			vector_add(children, $3);
			$$ = make_tree(ARRAY, children);
		}
		;

procedure_statement:
				   IDENT 
				   {
				   	$$ = $1;
				   }
				   |
				   IDENT OBRACK expr_list EBRACK
				   {
						fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!UNWRITTEN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				   }
				   ;

expr_list:
		 expr
		 {
		 	vector* children = vector_malloc();
			vector_add(children, $1);
			$$ = make_tree(EXPRESSION_LIST, children);
		 }
		 |
		 expr_list COMMA expr
		 {
			vector_add($1->children, $3);
			$$ = $1;
		 }
		 ;

expr:
	 simple_expr
	 {
	 	$$ = $1;
	 	fprintf(stderr, "EXPR-----------------------------------------------\n");
		fprintf(stderr, "DOLLAR 1:\n");
		print_tree($1, 0);

		fprintf(stderr, "DOLLAR DOLLAR:\n");
		print_tree($$, 0);
		fprintf(stderr, "-------------------------------------------------\n");
	 }
	 |
	 simple_expr RELOP simple_expr
	 {
		vector_add($2->children, $1);
		vector_add($2->children, $3);
		$$ = $2;
	 }
	 ;

simple_expr:
	term
	{
		$$ = $1;
	}
	|
	simple_expr SIGN term
	{
	 	fprintf(stderr, "SIMPLE_EXPR-----------------------------------------------\n");
		fprintf(stderr, "DOLLAR 1:\n");
		print_tree($1, 0);
		fprintf(stderr, "DOLLAR 3:\n");
		print_tree($3, 0);

		vector_add($2->children, $1);
		vector_add($2->children, $3);
		$$ = $2;
		fprintf(stderr, "DOLLAR DOLLAR:\n");
		print_tree($$, 0);
		fprintf(stderr, "-------------------------------------------------\n");
	}
	|
	simple_expr OR term
	{
		vector* children = vector_malloc();
		vector_add(children, $1);
		vector_add(children, $3);
		$$ = make_tree(OR, children);
	}
	;

term:
	 factor
	 {
	 	fprintf(stderr, "TERM-----------------------------------------------\n");
		fprintf(stderr, "DOLLAR 1:\n");
		print_tree($1, 0);

	  	$$ = $1;

		fprintf(stderr, "DOLLAR DOLLAR:\n");
		print_tree($$, 0);
		fprintf(stderr, "-------------------------------------------------\n");
	 }
	 |
	 SIGN factor
	 {
		vector_add($1->children, $2);
		$1->type=UNARY_SIGN;
		$$ = $1;
	 }
	 |
	 term MULOP factor
	 {
	 	fprintf(stderr, "TERM-----------------------------------------------\n");
		fprintf(stderr, "DOLLAR 1:\n");
		print_tree($1, 0);
		fprintf(stderr, "DOLLAR 3:\n");
		print_tree($3, 0);

		vector_add($2->children, $1);
		vector_add($2->children, $3);
		$$ = $2;

		fprintf(stderr, "DOLLAR DOLLAR:\n");
		print_tree($$, 0);
		fprintf(stderr, "-------------------------------------------------\n");
	 }
	 ;

factor:
	  IDENT
	  {
	  	printf("\t->Found an identifier: %s\n", $1->attribute.name);
		$$ = $1;
	  }
	  |
	  IDENT OSBRACK simple_expr ESBRACK
	  {
	  	printf("\t->Found an array: %s\n", $1->attribute.name);
		vector* children = vector_malloc();
		vector_add(children, $1);
		vector_add(children, $3);
		$$ = make_tree(ARRAY, children);
	  }
	  |
	  IDENT OBRACK expr_list EBRACK
	  {
	  	fprintf(stderr, "Found a function call: %s\n", $1->attribute.name);
		vector* children = vector_malloc();
		vector_add(children, $1);
		vector_add(children, $3);
		$$ = make_tree(FUNCTION_CALL, children);
	  }
	  |
	  NUM
	  {
	  	printf("\t->Found a number: %d\n", $1->attribute.ival);
		$$ = $1;
	  }
	  |
	  OBRACK expr EBRACK
	  {
	 	fprintf(stderr, "FACTOR: (EXPR)-----------------------------------------------\n");
		fprintf(stderr, "DOLLAR 2:\n");
		print_tree($2, 0);

	  	$$ = $2;

		fprintf(stderr, "DOLLAR DOLLAR:\n");
		print_tree($$, 0);
		fprintf(stderr, "-------------------------------------------------\n");
	  }
	  |
	  NOT factor
	  {
		vector* children = vector_malloc();
		vector_add(children, $2);
		$$ = make_tree(NOT, children);
	  }
	  ;

%%
