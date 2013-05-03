#ifndef SCOPE_H
#define SCOPE_H

typedef struct tree_s tree_t;

#include "vector.h"
#include "tree.h"
#include <glib-2.0/glib.h>

#define HASH_TABLE_SIZE 211


typedef struct scope_s {
	GSList* map[HASH_TABLE_SIZE];
	int depth;
	struct scope_s* parent;
	char* scope_id;
}
scope;

typedef struct scope_e {
	char* name;
	tree_t* type;
	int retval;
	tree_t* scope_base;
}
elem;


void hash_init();
unsigned int hash_pjw (const void *x, unsigned int tablesize);
void hash_insert(tree_t* t, tree_t* type, int rettype, scope* table, tree_t* scope_base);
void semantic_error(char *str);
void assert_new(gpointer data, gpointer user_data);
gint compare(gconstpointer a, gconstpointer b);
elem* find_ident(const char* str, scope* table);

void print_scope(scope* scope, char* name);
void print_list(GSList* list);
void print_elem(gpointer data, gpointer user_data);
int get_ident_type(tree_t* t);
void pad_spaces(int pad, FILE* stream);

void semantic_check(tree_t* t, scope* parent);
int eval_expr(tree_t* right, int type, scope* table, int* secondary_type);
elem* table_lookup(tree_t* left, scope* table);
void recursive_assignment_check(tree_t* left, tree_t* right, scope* parent);
#endif
