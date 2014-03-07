LEX=flex
LEXLIB=-lfl
CFLAGS=-DNDEBUG -std=c99 -ggdb

all:: capp

capp: lex.yy.o token-handler.o
	${CC} -o $@ $^ ${LEXLIB}

lex.yy.c: scanner.l
	${LEX} $<

clean::
	${RM} lex.yy.c lex.yy.o capp scanner.o *token-handler.o
	${RM} *~

.PHONY: all clean

