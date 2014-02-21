LEX=flex
LEXLIB=-lfl
CFLAGS=-DNDEBUG
CXXFLAGS=${CFLAGS}

all:: capp cxxapp

capp: lex.yy.o token-handler.o
	${CC} -o $@ $^ ${LEXLIB}

cxxapp: lex.yy.o cxx-token-handler.o
	${CXX} -o $@ $^ ${LEXLIB}

lex.yy.c: scanner.l
	${LEX} $<

clean::
	${RM} lex.yy.c lex.yy.o capp cxxapp scanner.o *token-handler.o
	${RM} *~

.PHONY: all clean

