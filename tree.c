#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tree.h"
#include "y.tab.h"
#include "vector.h"


tree_t *make_tree( int type, tree_t *left, tree_t *right )
{
	tree_t *t = (tree_t *)malloc( sizeof(tree_t) );
	assert( t != NULL );

	t->type = type;
	t->left = left;
	t->right = right;

	return t;
}

void print_tree( tree_t *t, int spaces )
{
	int i;

	if ( t == NULL ) return;

	for (i=0 ; i<spaces; i++)
		fprintf( stderr, " " );

	switch (t->type) {

	case SIGN:
		fprintf( stderr, "[OP:%c]", '+');
		break;

	case MULOP:
		fprintf( stderr, "[OP:%c]", '*');
		break;

	case NUM:
		fprintf( stderr, "[NUM:%d]", t->attribute.ival );
		break;

	case IDENT:
		fprintf( stderr, "[IDENT:%s]", t->attribute.name );
		break;

	default:
		fprintf( stderr, "[UNKNOWN]" );
	}
	fprintf( stderr, "\n" );

	print_tree( t->left, spaces+4 );
	print_tree( t->right, spaces+4 );
}

int eval_tree(tree_t *t)
{

	if ( t == NULL ) {
//		if(t->left == NULL || t->right == NULL) {
			fprintf(stderr, "Incomplete tree found!\n");
			exit(1);
//		}
	}

	switch (t->type) {
	case SIGN:
		return eval_tree(t->left) + eval_tree(t->right);
		break;

	case MULOP:
		return eval_tree(t->left) * eval_tree(t->right);
		break;

	case NUM:
		return t->attribute.ival;
		break;

	case IDENT:
		fprintf(stderr, "Identifiers can't be evaluated!\n");
		exit(1);
		break;

	default:
		fprintf(stderr, "Unknown type found!\n");
		exit(1);
	}
}
