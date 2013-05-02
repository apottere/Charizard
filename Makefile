DEBUG= -d
CFLAGS= -lfl
LFLAGS=-DYYSTYPE=tree_t*

all: lexical syntax
	gcc lex.yy.c y.tab.c tree.c vector.c -g -o Charizard $(CFLAGS) $(LFLAGS)

lexical:
	lex $(DEBUG) lexical.l

syntax:
	yacc $(DEBUG) -v syntax.y

clean:
	rm -f *.o y.tab.* lex.yy.c y.output out_test* Charizard
