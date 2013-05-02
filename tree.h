#ifndef TREE_H
#define TREE_H

#include "vector.h"

typedef struct tree_s {
	int type;
	union {
		int ival;	/* NUM */
		char* name;
	} attribute;
	vector* children;
}
tree_t;


tree_t *make_tree( int type, vector* children );
void	print_tree( tree_t *t, int spaces );

#endif

