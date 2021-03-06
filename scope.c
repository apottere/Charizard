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
//	fprintf(stderr, "Adding '%s' to table: \n", new->name);
	//print_tree(new->type, 0);

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
		char* prefix = "cowsay -d -f dragon-and-cow -W80 Your program is unacceptable! Failure: ";
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
			stype = "INTEGER         ";
			break;

		case REAL:
			stype = "REAL            ";
			break;

		case NONE:
			stype = "NONE            ";
			break;

		case INPUT:
			stype = "INPUT           ";
			break;

		case ARRAY:
			artype = get_array_type(edata->type);

			if(artype == INTEGER) {
				asprintf(&stype, "ARRAY OF INTEGER", edata->type->attribute.ival);

			} else if (artype == REAL) {

				asprintf(&stype, "ARRAY OF REAL   ", edata->type->attribute.ival);
			} else {
				stype = "ERROR          ";
			}
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

		case INPUT:
			rtype = "INPUT      ";
			break;

		default:
			rtype = "ERROR      ";

	}

	fprintf(stderr, "| ");
	fprintf(stderr, "%s", edata->name);
	int namepad = 42 - strlen(edata->name);
	pad_spaces(namepad, stderr);
	fprintf(stderr, "| %s   | %s   |\n", stype, rtype);
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
		int num = 0;
		while(temp->type == ARRAY) {
//			fprintf(stderr, "\nFound an array, looping the type...\n");
//			print_tree(temp, 0);
			temp = (tree_t*)vector_get( ((tree_t*)(vector_get(temp->children, 1)))->children, 0 );
			num++;
		}
		retval = temp->type;

		t->attribute.ival = num;

	}

	return retval;

}



void print_scope(scope* scope, char* name) {

//	fprintf(stderr, "Printing scope!\n");
	fprintf(stderr, " ________________________________________________________________________________\n");
	fprintf(stderr, "| TABLE: %s, %d", name, scope->depth);
	int namepad = 32 - strlen(name);
	pad_spaces(namepad, stderr);
	fprintf(stderr, "| RETURN             | TYPE          |\n");

	int i;
	for(i = 0; i < HASH_TABLE_SIZE; i++) {
		if(scope->map[i] != NULL) {
			//fprintf(stderr, "\t->found non-null list.\n");
			print_list(scope->map[i]);
		}
	}

	fprintf(stderr, "\n");

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
	tree_t* idl;
	vector*  children;
//	fprintf(stderr, "Looping through %d\n", t->type);
//	print_tree(t, 0);

	switch(t->type) {

		case PROGRAM:
			t->attribute.scope = (scope*) malloc(sizeof(scope));
			t->attribute.scope->depth = 0;
			t->attribute.scope->parent = NULL;
			t->attribute.scope->scope_id = "PROGRAM";
			hash_init(t->attribute.scope);
			
			idl = CHILD(t, 0);
			max = vector_count(idl->children);
			for(i = 0; i < max; i++) {
				hash_insert(CHILD(idl, i), make_tree(INPUT, NULL), INPUT, t->attribute.scope, NULL);
			}

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

		case PROCEDURE:
		case FUNCTION:
			
			ename = (t->type == PROCEDURE)? PROCEDURE : FUNCTION;

//			fprintf(stderr, "Init scoping for FUNC/PROC\n");
//			print_tree(t, 0);
			t->attribute.scope = (scope*) malloc(sizeof(scope));
			t->attribute.scope->depth = parent->depth + 1;
			t->attribute.scope->parent = parent;
			t->attribute.scope->scope_id = strdup(CHILD( CHILD(t, 0), 0)->attribute.name);
			hash_init(t->attribute.scope);


			new_parent = t->attribute.scope;

//			fprintf(stderr, "Function '%s' of type:\n", CHILD( CHILD(t, 0), 0)->attribute.name);
			//print_tree(t, 0);
			hash_insert( CHILD( CHILD(t, 0), 0), (ename == FUNCTION) ? CHILD( CHILD( CHILD(t, 0), 2), 0) : make_tree(NONE, NULL), ename, parent , t);

			break;
	} 






	max = vector_count(t->children);
	for(i = 0; i < max; i++) {
		init_scoping((tree_t*)vector_get(t->children, i), new_parent == NULL? parent : new_parent);
	}
}


// BEGIN SEMANTIC CHECKING (cause i hate makefiles)


void semantic_check(tree_t* t, scope* parent) {

	scope* new_parent = NULL;
	int* second = NULL;
	int i;
	int j;
	int k;
	int l;
	int max;
	char* buf;
	tree_t* n;
	tree_t* m;
	tree_t* c;
	elem* e;
	vector* v;
	vector* w;
	int v_type;
	int w_type;
	int count;
	int type_t2;
	//fprintf(stderr, "Checking tree: \n");
	//print_tree(t, 0);

	switch(t->type) {

		case PROCEDURE:
			break;

		case FUNCTION:

			check_function_return_statement(t, t->attribute.scope);

			new_parent = t->attribute.scope;
			break;

		case ASSIGNOP:
			//fprintf(stderr, "ASSIGNOP: \n");
			recursive_assignment_check(CHILD(t, 0), CHILD(t,1), parent);
			break;

		case IF:
		case ELSEIF:
		case WHILE:
			//fprintf(stderr, "BOOLEAN BLOCK: \n");
			n = CHILD(t, 0);
			if(n->type != RELOP) {
				if(n->type != NOT) {
					if(!(n->type == MULOP && my_strcmp(n->attribute.name, "and"))) {
						if(!(n->type == OR && my_strcmp(n->attribute.name, "or"))) {
							asprintf(&buf, "Boolean statement must be boolean at highest level [at least].");
							semantic_error(buf);
						}
					}
				}
			}

			eval_expr(n, BOOLEAN, parent, NULL);
			break;

		case FOR:
			//fprintf(stderr, "FOR BLOCK: \n");
			e = table_lookup(CHILD( CHILD(t, 0), 0), parent);
			if(!e->type->type == INTEGER) {
				asprintf(&buf, "Float variable in for loop declaration: %s", e->name);
				semantic_error(buf);
			}

			eval_expr(CHILD( CHILD(t, 0), 1), INTEGER, parent, NULL);
			eval_expr( CHILD(t, 1), INTEGER, parent, NULL);
			break;


		case ARRAY_RANGE:
			n = CHILD(t, 0);
			m = CHILD(t, 1);

			if(n->type == REAL) {
				asprintf(&buf, "Float found in array range declaration: %f", n->attribute.fval);
				semantic_error(buf);
			}

			if(m->type == REAL) {
				asprintf(&buf, "Float found in array range declaration: %f", m->attribute.fval);
				semantic_error(buf);
			}

			if(m->attribute.ival < n->attribute.ival) {
				asprintf(&buf, "Invalid array range declaration: %d..%d", n->attribute.ival, m->attribute.ival);
				semantic_error(buf);
			}
			break;

		case ARRAY_ACCESS:
			eval_expr(CHILD(t, 1), INTEGER, parent, NULL);
			break;

		case FUNCTION_CALL:
		case PROCEDURE_CALL:
			
			e = table_lookup(CHILD(t, 0), parent);
			if(e == NULL) {
				asprintf(&buf, "No table entry for function %s... this is bad.", CHILD(t, 0)->attribute.name);
				semantic_error(buf);
			}
			n = e->scope_base;
//			fprintf(stderr, "Exploring function call: %s\n", CHILD(t, 0)->attribute.name);

			m = CHILD( CHILD( n, 0 ), 1 );
			n = CHILD( t, 1 );

//			print_tree(m, 0);
//			print_tree(n, 0);

			//v = vector_malloc();
			w = n->children;

			count = 0;
			k = vector_count(m->children);
//			fprintf(stderr, "Looping %d times.\n", k);
			for(i = 0; i < k; i++) {
//				fprintf(stderr, "loop\n");
				v = CHILD(CHILD(m, i), 0)->children;
				v_type = CHILD(CHILD(CHILD(m, i), 1), 0)->type;


				l = vector_count(v);
				for(j = 0; j < l; j++) {
					if(vector_count(w) == count) {
						asprintf(&buf, "Too few arguments for function %s.", CHILD(t, 0)->attribute.name);
						semantic_error(buf);
					}
					c = (tree_t*) vector_get(w, count);
					if(c->type == ARRAY_ACCESS) {
						c = CHILD(c, 0);
					}
					e = table_lookup(c, parent);
					if(e == NULL) {
						asprintf(&buf, "No table entry for function %s... this is bad.", c->attribute.name);
						semantic_error(buf);
					}
					if(e->type->type == ARRAY) {
						type_t2 = get_array_type(e->type);
					} else {
						type_t2 = e->type->type;
					}
					if(!(type_t2 == v_type)) {
						asprintf(&buf, "Type mismatch with argument %d to function %s.", count + 1, CHILD(t, 0)->attribute.name);
						semantic_error(buf);
					}
					count++;
				}
			}
			if(vector_count(w) > count) {
				asprintf(&buf, "Too many arguments for function %s.", CHILD(t, 0)->attribute.name);
				semantic_error(buf);

			}
//			fprintf(stderr, "Done.\n");
			/*
			if(((tree_t*)vector_get(w, count)) != NULL) {
						asprintf(&buf, "Too many arguments to function %s.", c->attribute.name);
						semantic_error(buf);
			for(i = 0; i < vector_count(CHILD(m, 0)->children); i++) {
				w = CHILD(m, i)->children;
				for(j = 0; j < vector_count(w); j++) {
					vector_add(v, vector_get(w, j));
				}
				i++;
			}
			fprintf(stderr, "Found %d arguments to function %s.\n", vector_count(v), CHILD(t, 0)->attribute.name);
			w = n->children;
			if(vector_count(w) < vector_count(v)) {
				asprintf(&buf, "Too few arguments to function %s.", c->attribute.name);
				semantic_error(buf);
			}

			if(vector_count(v) < vector_count(w)) {
				asprintf(&buf, "Too many arguments to function %s.", c->attribute.name);
				semantic_error(buf);
			}
			
			for(i = 0; i < vector_count(v); i++) {

			}
			*/

			/*
			while(CHILD(m, i) != NULL) {
				v = CHILD(CHILD(m, i), 0)->children;
				v_type = CHILD(CHILD(m, i), 1)->type;


				for(j = 0; j < vector_count(v); j++) {
					c = (tree_t*)vector_get(w, count);
					fprintf(stderr, "Ident: %s, type: %s", c->attribute.name, v_type);
					if(c == NULL) {
					}
					e = table_lookup(c, parent);
					if(e == NULL) {
						asprintf(&buf, "No table entry for function %s... this is bad.", c->attribute.name);
						semantic_error(buf);
					}
					if(!e->type->type == v_type) {
						asprintf(&buf, "Type mismatch with argument %d to function %s.", count + 1, c->attribute.name);
						semantic_error(buf);
					}
					count++;
				}
			}
			if(((tree_t*)vector_get(w, count)) != NULL) {
						asprintf(&buf, "Too many arguments to function %s.", c->attribute.name);
						semantic_error(buf);
			} */


			break;

		//TODO: temp list to help me think.
		case PROGRAM:
		case DECLARATION_LIST:
		case DECLARATION:
		case IDENTIFIER_LIST:
		case IDENT:
		case TYPE:
		case INTEGER:
		case REAL:
		case NUM:
		case FUNCTION_LIST:
		case FUNCTION_HEADER:
		case PARAMETER_LIST:
		case PARAMETER:
		case STATEMENT_LIST:
		case EXPRESSION_LIST:
		case IF_STATEMENT:
		case RELOP:
		case THEN:
		case ELSE:
		case COMPOUND_STATEMENT:
		case ELSEIF_LIST:
		case MULOP:
		case UNARY_SIGN:
		case FLOAT:
		case SIGN:
		case NOT:
		case ARRAY:
		case PROCEDURE_HEADER:
			break;

		default:
			fprintf(stderr, "Unfinished semantic check rule found: %d\n", t->type);
			print_tree(t, 0);
			exit(1);


	}

	max = vector_count(t->children);
	for(i = 0; i < max; i++) {
		semantic_check(CHILD(t, i), new_parent == NULL? parent : new_parent);
	}
}



void recursive_assignment_check(tree_t* left, tree_t* right, scope* parent) {

	int glob_type = NULL;

	switch(left->type) {
		case IDENT:
			break;
		case ARRAY_ACCESS:
			left = CHILD(left, 0);
			break;

		default:
			semantic_error("Only identifiers are allowed to the left of an assignment.");
	}


	elem* e = table_lookup(left, parent);

	if(e == NULL) {
		char* err;
		asprintf(&err, "Table lookup failed for: '%s'", left->attribute.name);
		semantic_error(err);

	} else if(e->retval == FUNCTION) {

		if(!my_strcmp(e->name, parent->scope_id)) {
			char* err;
			asprintf(&err, "Non-current function id on left of assignment: '%s'", left->attribute.name);
			semantic_error(err);
		}

		glob_type = e->type->type;
		//fprintf(stderr, "\t->type: ");
		//print_tree(e->type, 0);

	} else if(e->retval == PROCEDURE) {
		char* err;
		asprintf(&err, "Procedures are not allowed on left of assignment: '%s'", left->attribute.name);
		semantic_error(err);
		
	} else {
		if(e->type->type == ARRAY) {
			glob_type = get_array_type(e->type);
		} else {
			glob_type = e->type->type;
		}
		//fprintf(stderr, "\t->type: ");
		//print_tree(e->type, 0);

	}



	eval_expr(right, glob_type, parent, NULL);
	return;

}

my_strcmp(char* s1, char* s2) {

	if(strlen(s1) == strlen(s2) && !strcmp(s1, s2)) {
		return 1;
	}
	return 0;

}


elem* table_lookup(tree_t* left, scope* table) {

	elem* e = NULL;
	int i;

	for(i = 0; i < HASH_TABLE_SIZE; i++) {
		if(table->map[i] != NULL) {

			GSList* ptr = table->map[i];
			while(ptr != NULL) {

				if(strlen(((elem*)ptr->data)->name) == strlen(left->attribute.name) && !strcmp(((elem*)ptr->data)->name, left->attribute.name)) {
					e = (elem*)ptr->data;
					//fprintf(stderr, "\t->Table lookup success! ID: %s\n", e->name);
					/*
				} else {
					fprintf(stderr, "'%s' != '%s'", ((elem*)ptr->data)->name, left->attribute.name);
					*/
				}

				ptr = g_slist_next(ptr);
			}
		}
	}
	
	if(e == NULL && table->parent != NULL) {
		e = table_lookup(left, table->parent);
	}

	
	return e;
}


int eval_expr(tree_t* right, int type, scope* table, int* secondary_type) {

	int i;
	int max;
	int ret = 0;
	elem* e;
	char* buf;
	char* op;
	int artype;

	switch(right->type) {
		case SIGN:
			/*
			if(type == BOOLEAN) {
				semantic_error("Addition/subtraction found in boolean expression.");
			}
			*/
			break;

		case NUM:
			if(type == REAL) {
				asprintf(&buf, "Integer found in float expression: %d", right->attribute.ival);
				semantic_error(buf);

			} else if(type == BOOLEAN) {
				if(secondary_type != NULL) {
					if(*secondary_type == REAL) {
						asprintf(&buf, "Integer found in boolean(float) expression: %d", right->attribute.ival);
						semantic_error(buf);
					}
				} else {
					secondary_type = (int*) malloc(sizeof(int));
					*secondary_type = INTEGER;
				}
			}


			break;

		case FLOAT:
			if(type == INTEGER) {
				asprintf(&buf, "Float found in integer expression: %f", right->attribute.fval);
				semantic_error(buf);

			} else if(type == BOOLEAN) {
				if(secondary_type != NULL){
					if(*secondary_type == INTEGER) {
						asprintf(&buf, "Float found in boolean(INTEGER) expression: %f", right->attribute.fval);
						semantic_error(buf);
					}
				} else {
					secondary_type = (int*) malloc(sizeof(int));
					*secondary_type = FLOAT;
				}

			}
			break;

		case RELOP:
			if(type == INTEGER) {

				asprintf(&buf, "Relational operator found in integer expression: %s", right->attribute.name);
				semantic_error(buf);

			} else if(type == REAL) {

				asprintf(&buf, "Relational operator found in float expression: %s", right->attribute.name);
				semantic_error(buf);
			}
			break;

		case UNARY_SIGN:
			/*
			if(type == BOOLEAN) {
				asprintf(&buf, "Unary sign operator found in boolean expression: %s", right->attribute.name);
				semantic_error(buf);
			} */
			break;

		case FUNCTION_CALL:
			//print_tree(right, 0);
			e = table_lookup(CHILD(right, 0), table);

			if(e->type->type == NONE) {
				asprintf(&buf, "Invalid procedure usage in expression: %s", e->name);
				semantic_error(buf);
			}

			if(e->type->type != type && type != BOOLEAN) {
				asprintf(&buf, "Invalid function return type in expression: %s", e->name);
				semantic_error(buf);

			} else if(type == BOOLEAN) {
				if(secondary_type != NULL){
					if(*secondary_type != e->type->type) {
						asprintf(&buf, "Invalid function return type in boolean expression: %s", e->name);
						semantic_error(buf);
					}
				} else {
					secondary_type = (int*) malloc(sizeof(int));
					*secondary_type = e->type->type;
				}

			}
			return ret;
			break;

		case IDENT:

			e = table_lookup(right, table);
			if(e->type->type != type && type != BOOLEAN) {
				asprintf(&buf, "Invalid variable type in expression: %s", e->name);
				semantic_error(buf);

			} else if(type == BOOLEAN) {
				if(secondary_type != NULL){
					if(*secondary_type != e->type->type) {
						asprintf(&buf, "Invalid variable type in boolean expression: %s", e->name);
						semantic_error(buf);
					}
				} else {
					secondary_type = (int*) malloc(sizeof(int));
					*secondary_type = e->type->type;
				}

			}
			break;

		case MULOP:

			op = strdup(right->attribute.name);
			if(my_strcmp(op, "*") || my_strcmp(op, "/")) {
				/*
				if(type == BOOLEAN) {
					asprintf(&buf, "Multiplication operator found in boolean expression: %s", op);
					semantic_error(buf);

				} */
			} else if(my_strcmp(op, "div") || my_strcmp(op, "div")) {
				if(type == REAL) {
					asprintf(&buf, "Integer division operator found in float expression: %s", op);
					semantic_error(buf);
				} else if(type == BOOLEAN) {
					if(secondary_type != NULL) {
						if(*secondary_type == REAL) {
							asprintf(&buf, "Integer division operator found in boolean(float) expression: %s", op);
							semantic_error(buf);
						}

					} else {
						secondary_type = (int*) malloc(sizeof(int));
						*secondary_type = INTEGER;
					}

				}

			} else if(my_strcmp(op, "and")) {

				if(type == REAL) {
					asprintf(&buf, "Logical and operator found in real expression.");
					semantic_error(buf);
				} else if(type == INTEGER) {
					asprintf(&buf, "Logical and operator found in integer expression.");
					semantic_error(buf);
				}

			}

			break;

		case NOT:
			if(type == REAL) {
				asprintf(&buf, "Logical not operator found in float expression.");
				semantic_error(buf);
			} else if(type == INTEGER) {
				asprintf(&buf, "Logical not operator found in INTEGER expression.");
				semantic_error(buf);
			}

			break;

		case ARRAY_ACCESS:

			e = table_lookup(CHILD(right, 0), table);
			artype = get_array_type(e->type);
			//fprintf(stderr, "Checking array of type: %d agains env %d", artype, type);
			if(artype != type && type != BOOLEAN) {
				asprintf(&buf, "Invalid array type in expression: %s", e->name);
				semantic_error(buf);

			} else if(type == BOOLEAN) {
				if(secondary_type != NULL){
					if(*secondary_type != artype) {
						asprintf(&buf, "Invalid array type in boolean expression: %s", e->name);
						semantic_error(buf);
					}
				} else {
					secondary_type = (int*) malloc(sizeof(int));
					*secondary_type = artype;
				}

			}
			return ret;
			break;

		default:
			fprintf(stderr, "Unfinished expression check rule found: %d\n", right->type);
			print_tree(right, 0);
			exit(1);
	}

	max = vector_count(right->children);
	for(i = 0; i < max; i++) {
		ret += eval_expr(CHILD(right, i), type, table, NULL);
	}
	return ret;

}


void check_function_return_statement(tree_t* t, scope* parent) {

	tree_t* id = CHILD( CHILD(t, 0), 0);
	int ret = 0;
	char* buf;
	scope* backup = parent->parent;
	parent->parent = NULL;

	check_function_return_statement_helper(CHILD(t, vector_count(t->children) - 1), parent, &ret, id);
	if(!ret) {
		asprintf(&buf, "Function does not return: %s", id->attribute.name);
		semantic_error(buf);
	}
	parent->parent = backup;
}

void check_function_return_statement_helper(tree_t* t, scope* parent, int* code, tree_t* target) {

	int i, max;
	char* buf;
	
	if(t->type == ASSIGNOP) {
		if(CHILD(t, 0)->type == IDENT) {
			if(my_strcmp(target->attribute.name, CHILD(t, 0)->attribute.name)) {
				*code = 1;
			} else {
				elem* e = table_lookup(CHILD(t, 0), parent);
				if(e == NULL) {
					asprintf(&buf, "Function %s alters variable out of scope: %s", target->attribute.name, CHILD(t, 0)->attribute.name);
					semantic_error(buf);
				}
			} 
		}
	}

	
	max = vector_count(t->children);
	for(i = 0; i < max; i++) {
		check_function_return_statement_helper(CHILD(t, i), parent, code, target);
	}
}

