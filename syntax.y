%code requires
{
#include "tree.h"
#include "vector.h"
}

%error-verbose

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define YYSTYPE tree_t*
 
void yyerror(const char *str)
{
		char* prefix = "cowsay -d -f dragon-and-cow -W57 Your program is unacceptable!       Failure: ";
		char* error = (char*) malloc(sizeof(char)*(strlen(prefix) + strlen(str) + 2));
		sprintf(error, "%s%s\n",prefix, str);
        if(system(error)) {
			fprintf(stderr, "Install cowsay already! %s\n", str);
		}
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

%token IDENT NUM INTEGER REAL FLOAT
%token OBRACK EBRACK OSBRACK ESBRACK
%token MULOP SIGN RELOP
%token NOT OR ASSIGNOP
%token WHILE DO IF THEN ELSEIF ELSE BEGIN_T END ELSEIF_LIST
%token COLON SEMICOLON COMMA DOT
%token PROGRAM FUNCTION ARRAY OF VAR PROCEDURE

// Too lazy to make my own enumeration!
%token RESERVED STATEMENT_LIST FUNCTION_CALL EXPRESSION_LIST UNARY_SIGN DECLARATION DECLARATION_LIST IDENTIFIER_LIST ARRAY_TYPE FUNCTION_LIST TYPE PARAMETER PARAMETER_LIST FUNCTION_HEADER PROCEDURE_HEADER PROCEDURE_CALL COMPOUND_STATEMENT FOR TO IF_STATEMENT ARRAY_RANGE NONE

%%

program:
	   PROGRAM IDENT OBRACK identifier_list EBRACK SEMICOLON declarations subprogram_declarations compound_statement DOT
	   {
	   	//fprintf(stderr, "Program found:\n");
		vector* children = vector_malloc();
		vector_add(children, $7);
		vector_add(children, $8);
		vector_add(children, $9);
		tree_t* final = make_tree(PROGRAM, children);
		print_tree(final, 0);
		init_scoping(final, NULL);
		print_scope(final->attribute.scope, "PROGRAM");

		if(system("cowsay -f dragon I approve of your program.")) {
			printf("Install cowsay, or reap the consequences!\n");
			exit(1);
		}
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
		$$ = $1;
	}
	|
	ARRAY OSBRACK array_range ESBRACK OF standard_type
	{
		vector* children = vector_malloc();
		vector_add(children, $3);
		vector_add(children, $6);
		vector* children2 = vector_malloc();
		vector_add(children2, make_tree(ARRAY, children));
		$$ = make_tree(TYPE, children2);
	}
	;

array_range:
		   NUM DOT DOT NUM
		   {
			vector* children = vector_malloc();
			vector_add(children, $1);
			vector_add(children, $4);
			$$ = make_tree(ARRAY_RANGE, children);
		   }
		   |
		   NUM
		   {
			vector* children = vector_malloc();

			tree_t* new = make_tree(NUM, NULL);
			new->attribute.ival = 1;

			vector_add(children, new);
			vector_add(children, $1);
			$$ = make_tree(ARRAY_RANGE, children);
		   }
		   ;


standard_type:
			 INTEGER
			 {
				vector* children = vector_malloc();
				vector_add(children, make_tree(INTEGER, NULL));
				$$ = make_tree(TYPE, children);
			 }
			 |
			 REAL
			 {
				vector* children = vector_malloc();
				vector_add(children, make_tree(REAL, NULL));
				$$ = make_tree(TYPE, children);
			 }
			 ;

subprogram_declarations:
					   subprogram_declarations subprogram_declaration
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
			  statement
			  {
				//fprintf(stderr, "TERM-----------------------------------------------\n");
				//fprintf(stderr, "DOLLAR 1:\n");
				//print_tree($1, 0);

			  	vector* children = vector_malloc();
				vector_add(children, $1);
			  	$$ = make_tree(STATEMENT_LIST, children);

				//fprintf(stderr, "DOLLAR DOLLAR:\n");
				//print_tree($$, 0);
				//fprintf(stderr, "-------------------------------------------------\n");
			  }
			  |
			  statement_list statement
			  {
			  	vector_add($1->children, $2);
				$$ = $1;
			  }
			  ;

statement:
		 assign_statement SEMICOLON
		 {
		 	$$ = $1;
		 }
		 |
		 procedure_statement SEMICOLON
		 {
		 	$$ = $1;
		 }
		 |
		 compound_statement
		 {
		 	
			vector* children = vector_malloc();
			vector_add(children, $1);
			$$ = make_tree(COMPOUND_STATEMENT, children);
		 }
		 |
		 IF expr THEN statement elseif else END
		 {
			vector* children = vector_malloc();
			vector* children2 = vector_malloc();
			vector_add(children2, $2);
			vector_add(children, make_tree(IF, children2));

			vector* children3 = vector_malloc();
			vector_add(children3, $4);
			vector_add(children, make_tree(THEN, children3));

			if($5 != NULL) {
				vector_add(children, $5);
			}

			if($6 != NULL) {
				vector_add(children, $6);
			}

			$$ = make_tree(IF_STATEMENT, children);
		 }
		 |
		 WHILE expr DO statement
		 {
			vector* children = vector_malloc();
			vector_add(children, $2);
			vector_add(children, $4);
			$$ = make_tree(WHILE, children);
		 }
		 |
		 FOR assign_statement TO expr DO statement
		 {
			vector* children = vector_malloc();
			vector_add(children, $2);
			vector_add(children, $4);
			vector_add(children, $6);
			$$ = make_tree(FOR, children);
		 }
		 ;

assign_statement:
		 variable ASSIGNOP expr
		 {
			//fprintf(stderr, "STATEMENT: ASSIGN-----------------------------------------------\n");
			//fprintf(stderr, "DOLLAR 1:\n");
			//print_tree($1, 0);
			//fprintf(stderr, "DOLLAR 3:\n");
			//print_tree($3, 0);

		 	//fprintf(stderr, "FOUND ASSIGN OP\n");
			vector* children = vector_malloc();
			vector_add(children, $1);
			vector_add(children, $3);
		 	$$ = make_tree(ASSIGNOP, children);

			//fprintf(stderr, "DOLLAR DOLLAR:\n");
			//print_tree($$, 0);
			//fprintf(stderr, "-------------------------------------------------\n");
		 }
		 ;

else:
	ELSE statement
	{
		vector* children = vector_malloc();
		vector_add(children, $2);
		$$ = make_tree(ELSE, children);
	}
	|
	{
		$$ = NULL;
	}
	;

elseif:
	  elseif ELSEIF expr THEN statement
	  {
		vector* children = vector_malloc();
		vector_add(children, $3);
		vector_add(children, $5);

		if($1 == NULL) {

			vector* children2 = vector_malloc();
			vector_add(children2, make_tree(ELSEIF, children));

			$$ = make_tree(ELSEIF_LIST, children2);

		} else {
			vector_add($1->children, make_tree(ELSEIF, children));
			$$ = $1;
		}
	  }
	  |
	  {
	  	$$ = NULL;
	  }
	  ;

variable:
		IDENT
		{
			//fprintf(stderr, "VARIABLE-----------------------------------------------\n");
			//fprintf(stderr, "DOLLAR 1:\n");
			//print_tree($1, 0);

			$$ = $1;

			//fprintf(stderr, "DOLLAR DOLLAR:\n");
			//print_tree($$, 0);
			//fprintf(stderr, "-------------------------------------------------\n");
		}
		|
		IDENT OSBRACK simple_expr ESBRACK
		{
			//printf("\t->Found an array: %s\n", $1->attribute.name);
			vector* children = vector_malloc();
			vector_add(children, $1);
			vector_add(children, $3);
			$$ = make_tree(ARRAY, children);
		}
		;

procedure_statement:
				   IDENT 
				   {
						vector* children = vector_malloc();
						vector_add(children, $1);
						$$ = make_tree(PROCEDURE_CALL, children);
				   }
				   |
				   IDENT OBRACK expr_list EBRACK
				   {
						vector* children = vector_malloc();
						vector_add(children, $1);
						vector_add(children, $3);
						$$ = make_tree(PROCEDURE_CALL, children);
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
	 	//fprintf(stderr, "EXPR-----------------------------------------------\n");
		//fprintf(stderr, "DOLLAR 1:\n");
		//print_tree($1, 0);

		//fprintf(stderr, "DOLLAR DOLLAR:\n");
		//print_tree($$, 0);
		//fprintf(stderr, "-------------------------------------------------\n");
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
	 	//fprintf(stderr, "SIMPLE_EXPR-----------------------------------------------\n");
		//fprintf(stderr, "DOLLAR 1:\n");
		//print_tree($1, 0);
		//fprintf(stderr, "DOLLAR 3:\n");
		//print_tree($3, 0);

		vector_add($2->children, $1);
		vector_add($2->children, $3);
		$$ = $2;
		//fprintf(stderr, "DOLLAR DOLLAR:\n");
		//print_tree($$, 0);
		//fprintf(stderr, "-------------------------------------------------\n");
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
	 	//fprintf(stderr, "TERM-----------------------------------------------\n");
		//fprintf(stderr, "DOLLAR 1:\n");
		//print_tree($1, 0);

	  	$$ = $1;

		//fprintf(stderr, "DOLLAR DOLLAR:\n");
		//print_tree($$, 0);
		//fprintf(stderr, "-------------------------------------------------\n");
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
	 	//fprintf(stderr, "TERM-----------------------------------------------\n");
		//fprintf(stderr, "DOLLAR 1:\n");
		//print_tree($1, 0);
		//fprintf(stderr, "DOLLAR 3:\n");
		//print_tree($3, 0);

		vector_add($2->children, $1);
		vector_add($2->children, $3);
		$$ = $2;

		//fprintf(stderr, "DOLLAR DOLLAR:\n");
		//print_tree($$, 0);
		//fprintf(stderr, "-------------------------------------------------\n");
	 }
	 ;

factor:
	  IDENT
	  {
	  	//printf("\t->Found an identifier: %s\n", $1->attribute.name);
		$$ = $1;
	  }
	  |
	  IDENT OSBRACK simple_expr ESBRACK
	  {
	  	//printf("\t->Found an array: %s\n", $1->attribute.name);
		vector* children = vector_malloc();
		vector_add(children, $1);
		vector_add(children, $3);
		$$ = make_tree(ARRAY, children);
	  }
	  |
	  IDENT OBRACK expr_list EBRACK
	  {
	  	//fprintf(stderr, "Found a function call: %s\n", $1->attribute.name);
		vector* children = vector_malloc();
		vector_add(children, $1);
		vector_add(children, $3);
		$$ = make_tree(FUNCTION_CALL, children);
	  }
	  |
	  NUM
	  {
	  	//printf("\t->Found a number: %d\n", $1->attribute.ival);
		$$ = $1;
	  }
	  |
	  FLOAT
	  {
	  	//printf("\t->Found a float: %f\n", $1->attribute.fval);
		$$ = $1;
	  }
	  |
	  OBRACK expr EBRACK
	  {
	 	//fprintf(stderr, "FACTOR: (EXPR)-----------------------------------------------\n");
		//fprintf(stderr, "DOLLAR 2:\n");
		//print_tree($2, 0);

	  	$$ = $2;

		//fprintf(stderr, "DOLLAR DOLLAR:\n");
		//print_tree($$, 0);
		//fprintf(stderr, "-------------------------------------------------\n");
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
