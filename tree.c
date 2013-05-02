#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tree.h"
#include "y.tab.h"
#include "vector.h"

#define TREE_WIDTH 5


tree_t *make_tree( int type, vector* children )
{
	tree_t *t = (tree_t *)malloc( sizeof(tree_t) );
	assert( t != NULL );

	if(children == NULL) {

		vector *v = malloc(sizeof(vector));
		assert( v != NULL );
		vector_init(v);
		t->children = v;

	} else {
		t->children = children;
	}

	t->type = type;

	return t;
}

void print_tree( tree_t *t, int spaces )
{
	int i;

	if ( t == NULL ) return;

	for (i=0 ; i<spaces; i++) {
		if(!(i % TREE_WIDTH)) {
			fprintf( stderr, "|" );
		} else {
			fprintf( stderr, " " );
		}
	}

	switch (t->type) {

	case SIGN:
		fprintf( stderr, "[OP %c]", '+');
		break;

	case UNARY_SIGN:
		fprintf( stderr, "[SIGN %c]", '+');
		break;

	case MULOP:
		fprintf( stderr, "[OP %s]", t->attribute.name);
		break;

	case STATEMENT_LIST:
		fprintf( stderr, "[STATEMENT LIST]" );
		break;

	case FUNCTION:
		fprintf( stderr, "[FUNCTION]" );
		break;

	case FUNCTION_LIST:
		fprintf( stderr, "[FUNCTION LIST]" );
		break;

	case INTEGER:
		fprintf( stderr, "[INTEGER]" );
		break;

	case REAL:
		fprintf( stderr, "[REAL]" );
		break;

	case FOR:
		fprintf( stderr, "[FOR]" );
		break;

	case WHILE:
		fprintf( stderr, "[WHILE]" );
		break;

	case IF:
		fprintf( stderr, "[IF]" );
		break;

	case THEN:
		fprintf( stderr, "[THEN]" );
		break;

	case ELSE:
		fprintf( stderr, "[ELSE]" );
		break;

	case COMPOUND_STATEMENT:
		fprintf( stderr, "[COMPOUND STATEMENT]" );
		break;

	case PROCEDURE_CALL:
		fprintf( stderr, "[PROCEDURE CALL]" );
		break;

	case IFT:
		fprintf( stderr, "[IF THEN]" );
		break;

	case ELSEIF:
		fprintf( stderr, "[ELSE IF]" );
		break;

	case ELSEIF_LIST:
		fprintf( stderr, "[ELSE IF LIST]" );
		break;

	case RELOP:
		fprintf( stderr, "[RELOP: %s]", t->attribute.name);
		break;

	case IFTE:
		fprintf( stderr, "[IF THEN ELSE]" );
		break;

	case FUNCTION_HEADER:
		fprintf( stderr, "[FUNCTION HEADER]" );
		break;

	case PROCEDURE_HEADER:
		fprintf( stderr, "[PROCEDURE HEADER]" );
		break;

	case PARAMETER:
		fprintf( stderr, "[PARAMETER]" );
		break;

	case PARAMETER_LIST:
		fprintf( stderr, "[PARAMETER LIST]" );
		break;

	case TYPE:
		fprintf( stderr, "[TYPE]" );
		break;

	case EXPRESSION_LIST:
		fprintf( stderr, "[EXPRESSION LIST]" );
		break;

	case DECLARATION:
		fprintf( stderr, "[DECLARATION]" );
		break;

	case DECLARATION_LIST:
		fprintf( stderr, "[DECLARATION LIST]" );
		break;

	case IDENTIFIER_LIST:
		fprintf( stderr, "[IDENTIFIER LIST]" );
		break;

	case ARRAY:
		fprintf( stderr, "[ARRAY]" );
		break;

	case ARRAY_TYPE:
		fprintf( stderr, "[ARRAY TYPE]" );
		break;

	case PROGRAM:
		fprintf( stderr, "[PROGRAM]" );
		break;

	case NOT:
		fprintf( stderr, "[NOT]" );
		break;

	case FUNCTION_CALL:
		fprintf( stderr, "[FUNCTION CALL]" );
		break;

	case ASSIGNOP:
		fprintf( stderr, "[OP %s]", ":=" );
		break;

	case NUM:
		fprintf( stderr, "[NUM:%d]", t->attribute.ival );
		break;

	case FLOAT:
		fprintf( stderr, "[FLOAT:%f]", t->attribute.fval );
		break;

	case IDENT:
		fprintf( stderr, "[IDENT:%s]", t->attribute.name );
		break;

	default:
		fprintf( stderr, "[UNKNOWN: %d]", t->type );
	}
	fprintf( stderr, "\n" );

	for(i = 0; i < t->children->count; i++) {
		print_tree( vector_get(t->children, i), spaces+TREE_WIDTH );
	}
}
