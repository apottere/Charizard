#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tree.h"
#include "scope.h"
#include "y.tab.h"
#include "vector.h"



unsigned int hash_pjw (const void *x, unsigned int tablesize)
{
  const char *s = x;
  unsigned int h = 0;
  unsigned int g;

  while (*s != 0)
    {
      h = (h << 4) + *s++;
      if ((g = h & (unsigned int) 0xf0000000) != 0)
        h = (h ^ (g >> 24)) ^ g;
    }

  return (h % tablesize);
}

void hash_init(scope* table) {
	
	int i;
	for(i = 0; i < HASH_TABLE_SIZE; i++) {
		table->map[i] = NULL;
	}

}

void hash_insert(tree_t* t, tree_t* type_t, int rettype, scope* table) {


	char* str = strdup(t->attribute.name);
	int type = get_ident_type(type_t);
	int hash = hash_pjw(str, HASH_TABLE_SIZE);
	elem* new = (elem*) malloc(sizeof(elem));

	new->name = str;
	new->type = type;
	new->retval = rettype;

	if(table->map[hash] == NULL) {
		table->map[hash] = g_slist_alloc();
		table->map[hash]->data = new;

	} else {
		void (*check)(gpointer, gpointer) = &assert_new;
		g_slist_foreach(table->map[hash], check, new->name);
		table->map[hash] = g_slist_append(table->map[hash], new);
	}
}


elem* find_ident(const char* str, scope* table) {

	GSList* found = NULL;
	gint (*comp)(gconstpointer, gconstpointer) = &compare;
	int hash = hash_pjw(str, HASH_TABLE_SIZE);

	found = g_slist_find_custom(table->map[hash], str, (GCompareFunc) comp);

	if(found != NULL) {
		return found->data;
	}
	
	if(table->parent != NULL) {
		return find_ident(str, table->parent);
	}

	return NULL;

}

gint compare(gconstpointer a, gconstpointer b) {
	
	if((strlen(((elem*)a)->name) == strlen((char*)b)) && (strcmp(((elem*)a)->name, (char*)b) == 0)) {
		return 0;
	}
	return -1;

}

void assert_new(gpointer data, gpointer user_data) {
	
	if((strlen(((elem*)data)->name) == strlen((char*)user_data)) && (strcmp(((elem*)data)->name, (char*)user_data) == 0)) {
		char* prefix = "Attempting to redefine variable: ";
		char* buf = (char*) malloc(sizeof(char)*(strlen(prefix) + strlen((char*)user_data) + 2));
		sprintf(buf, "%s%s\n", prefix, (char*)user_data);
		semantic_error(buf);
	}

}


void semantic_error(char *str) {
		char* prefix = "cowsay -d -f dragon-and-cow -W57 Your program is unacceptable!       Failure: ";
		char* error = (char*) malloc(sizeof(char)*(strlen(prefix) + strlen(str) + 2));
		sprintf(error, "%s%s\n",prefix, str);
        if(system(error)) {
			fprintf(stderr, "Install cowsay already! %s\n", str);
		}
		exit(1);
}

void print_list(GSList* list, int spaces) {

	void (*print)(gpointer, gpointer) = &print_elem;
	g_slist_foreach(list, print, &spaces);

}

void print_elem(gpointer data, gpointer user_data) {
		
	int i;
	int* spaces = (int*) user_data;
	elem* edata = (elem*) data;
	for(i = 0; i < *spaces; i++) {
		fprintf(stderr, " ");
	}
	char* stype;
	switch (edata->type) {
		case INTEGER:
			stype = "INTEGER";
			break;

		case REAL:
			stype = "REAL   ";
			break;
	}

	fprintf(stderr, "| %s\t\t\t| %s\t\t\t| %d\t\t\t|\n", edata->name, stype, edata->type);

}

int get_ident_type(tree_t* t) {
	int retval;
	tree_t* temp;

	switch(t->type) {

		case INTEGER:
			retval = INTEGER;
			break;

		case REAL:
			retval = REAL;
			break;

		case ARRAY:
			temp = t;
			while(temp->type == ARRAY) {
				temp = (tree_t*)vector_get( ((tree_t*)(vector_get(temp->children, 0)))->children, 0 );
			}
			retval = temp->type;

	}
	return retval;

}



void print_scope(scope* scope, int spaces) {

//	fprintf(stderr, "Printing scope!\n");
	fprintf(stderr, "\n_________________________________________________________________________________\n");
	fprintf(stderr, "| IDENT\t\t\t| TYPE   \t\t\t| RETURN\t\t\t|\n");
//	fprintf(stderr, "\n_________________________________________________________________________________\n");

	int i;
	for(i = 0; i < HASH_TABLE_SIZE; i++) {
		if(scope->map[i] != NULL) {
			//fprintf(stderr, "\t->found non-null list.\n");
			print_list(scope->map[i], spaces);
//			fprintf(stderr, "\n_________________________________________________________________________________\n");
		}
	}

}

init_scoping(tree_t* t, scope* parent) {

	scope* new_parent = NULL;
	int i;
	int max;
	tree_t* type;
	vector*  children;

	switch(t->type) {

		case PROGRAM:
			t->attribute.scope = (scope*) malloc(sizeof(scope));
			t->attribute.scope->depth = 0;
			t->attribute.scope->parent = NULL;
			hash_init(t->attribute.scope);

			new_parent = t->attribute.scope;
			break;

		case DECLARATION:
			
			type = (tree_t*)vector_get(((tree_t*)vector_get(t->children, 1))->children, 0);
//			type->type = 5;
			children = ((tree_t*)vector_get(t->children, 0))->children;
			max = vector_count(children);

			for(i = 0; i < max; i++) {
//				fprintf(stderr, "Inserting ID: '%s' of type %s");
				hash_insert(((tree_t*)vector_get(children, i)), type, NONE, parent);
			}

	} 






	max = vector_count(t->children);
	for(i = 0; i < max; i++) {
		init_scoping((tree_t*)vector_get(t->children, i), new_parent == NULL? parent : new_parent);
	}
}
