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

void hash_insert(tree_t* t, tree_t* type_t, int rettype, scope* table, tree_t* scope_base) {


	char* str = strdup(t->attribute.name);
	int hash = hash_pjw(str, HASH_TABLE_SIZE);
	elem* new = (elem*) malloc(sizeof(elem));

	new->name = str;
	new->type = type_t;
	new->retval = rettype;
	new->scope_base = scope_base;
	fprintf(stderr, "Adding '%s' to table: \n", new->name);
	print_tree(new->type, 0);

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


gint compare_and_print(gconstpointer a, gconstpointer b) {
	
	vector* children = (vector*) b;
	elem* thing = (elem*) a;
	int i;
	int max = vector_count(children);
	for(i = 0; i < max; i++) {
		if(thing->retval == *((int*)vector_get(children, i))) {
//			fprintf(stderr, "Found something named '%s', attempting to print its scope.\n", thing->name);
//			print_tree(thing->scope_base, 0);
			print_scope(thing->scope_base->attribute.scope, thing->name);
		}
	}
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

void print_list(GSList* list) {

	void (*print)(gpointer, gpointer) = &print_elem;
	g_slist_foreach(list, print, 0);

}

void print_elem(gpointer data, gpointer user_data) {
		
	int i;
	elem* edata = (elem*) data;

	char* stype;
	int artype;
	switch (edata->type->type) {
		case INTEGER:
			stype = "INTEGER       ";
			break;

		case REAL:
			stype = "REAL          ";
			break;

		case ARRAY:
			artype = get_array_type(edata->type);
			stype = artype == INTEGER ? "ARRAY[INTEGER]" : (artype == REAL ? "ARRAY[REAL]   " : "ERROR         ");
			break;
	}

	char* rtype;
	switch(edata->retval) {
		case FUNCTION:
			rtype = "FUNCTION   ";
			break;

		case PROCEDURE:
			rtype = "PROCEDURE  ";
			break;

		case PARAMETER:
			rtype = "PARAMETER  ";
			break;

		case DECLARATION:
			rtype = "DECLARATION";
			break;

		default:
			rtype = "ERROR      ";

	}

	fprintf(stderr, "| ");
	fprintf(stderr, "%s", edata->name);
	int namepad = 42 - strlen(edata->name);
	pad_spaces(namepad, stderr);
	fprintf(stderr, "| %s    | %s    |\n", stype, rtype);
	//fprintf(stderr, "| %s\t\t\t| %s\t\t\t| %s\t\t\t|\n", edata->name, stype, rtype);

}

void pad_spaces(int pad, FILE* stream) {

	int i;
	for(i = 0; i < pad; i++) {

		fprintf(stream, " ");
	}

}


int get_array_type(tree_t* t) {

	int retval = 0;

	if(t->type == ARRAY) {
		tree_t* temp;
		temp = t;
		while(temp->type == ARRAY) {
//			fprintf(stderr, "\nFound an array, looping the type...\n");
//			print_tree(temp, 0);
			temp = (tree_t*)vector_get( ((tree_t*)(vector_get(temp->children, 1)))->children, 0 );
		}
		retval = temp->type;

	}

	return retval;

}



void print_scope(scope* scope, char* name) {

//	fprintf(stderr, "Printing scope!\n");
	fprintf(stderr, "\n ________________________________________________________________________________\n");
	fprintf(stderr, "| TABLE: %s", name);
	int namepad = 35 - strlen(name);
	pad_spaces(namepad, stderr);
	fprintf(stderr, "| RETURN            | TYPE           |\n");

	int i;
	for(i = 0; i < HASH_TABLE_SIZE; i++) {
		if(scope->map[i] != NULL) {
			//fprintf(stderr, "\t->found non-null list.\n");
			print_list(scope->map[i]);
		}
	}

	vector* searches = vector_malloc();
	int thing = FUNCTION;
	vector_add(searches, &thing);
	int thing2 = PROCEDURE;
	vector_add(searches, &thing2);

	for(i = 0; i < HASH_TABLE_SIZE; i++) {
		if(scope->map[i] != NULL) {
			gint (*comp)(gconstpointer, gconstpointer) = &compare_and_print;
			g_slist_find_custom(scope->map[i], searches, (GCompareFunc) comp);
		}
	}

}

init_scoping(tree_t* t, scope* parent) {

	scope* new_parent = NULL;
	int i;
	int max;
	int ename;
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

		case PARAMETER:
		case DECLARATION:
			
			ename = t->type == PARAMETER? PARAMETER : DECLARATION;
			type = (tree_t*)vector_get(((tree_t*)vector_get(t->children, 1))->children, 0);
			children = ((tree_t*)vector_get(t->children, 0))->children;
			max = vector_count(children);

			for(i = 0; i < max; i++) {

				//fprintf(stderr, "Inserting ID: '%s' of type:\n", ((tree_t*)vector_get(children, i))->attribute.name);
				//print_tree(type, 0);
				hash_insert(((tree_t*)vector_get(children, i)), type, ename, parent, NULL);
			}
			break;

//		case PROCEDURE:
		case FUNCTION:
			t->attribute.scope = (scope*) malloc(sizeof(scope));
			t->attribute.scope->depth = 1;
			t->attribute.scope->parent = parent;
			hash_init(t->attribute.scope);


			new_parent = t->attribute.scope;

			fprintf(stderr, "Function '%s' of type:\n", CHILD( CHILD(t, 0), 0)->attribute.name);
			print_tree(t, 0);
			hash_insert( CHILD( CHILD(t, 0), 0), CHILD( CHILD( CHILD(t, 0), 2), 0), FUNCTION, parent , t);

			break;
	} 






	max = vector_count(t->children);
	for(i = 0; i < max; i++) {
		init_scoping((tree_t*)vector_get(t->children, i), new_parent == NULL? parent : new_parent);
	}
}
