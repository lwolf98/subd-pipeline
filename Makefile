CC=g++
CFLAGS=-O0 -ggdb3 -Wall
OBJ=stack.o strop.o

subd: subd.cpp elements.cpp parse/lex.yy.c parse/stack.c parse/strop.c
	$(CC) $(CFLAGS) -o $@ $^

parse/lex.yy.c:
	+$(MAKE) -C parse

clean:
	rm -f subd
