#ifndef TREE_H
#define TREE_H

#include "vector.h"
#include "scope.h"


typedef struct tree_s {
	int type;
	union {
		int ival;	/* NUM */
		double fval;	/* FLOAT */
		char* name;
		scope* scope;
	} attribute;
	vector* children;
}
tree_t;


tree_t *make_tree( int type, vector* children );
void	print_tree( tree_t *t, int spaces );

#endif

