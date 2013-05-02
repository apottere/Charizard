PKGCONFIG=`pkg-config --cflags --libs glib-2.0`
DEBUG= -d
CFLAGS= -lfl
LFLAGS=-DYYSTYPE=tree_t*

all: lexical syntax
	gcc lex.yy.c y.tab.c scope.c tree.c vector.c $(PKGCONFIG) -g -o Charizard $(CFLAGS) $(LFLAGS)

lexical:
	lex $(DEBUG) lexical.l

syntax:
	yacc $(DEBUG) -v syntax.y

clean:
	rm -f *.o y.tab.* lex.yy.c y.output out_test* Charizard
