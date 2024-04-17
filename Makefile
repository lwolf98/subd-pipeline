CC=g++
CFLAGS=-O0 -ggdb3 -Wall
#OBJ=stack.o strop.o
OBJ=parse/stack.o parse/strop.o parse/obj.tab.o parse/lex.yy.o
SUBD=parse/stack.c parse/strop.c parse/obj.tab.c parse/lex.yy.c

#subd: subd.cpp elements.cpp parse/lex.yy.c parse/stack.c parse/strop.c
#	$(CC) $(CFLAGS) -o $@ $^
subd: subd.cpp elements.cpp $(SUBD)
	$(CC) $(CFLAGS) -o $@ $^ -lm

parse/lex.yy.o parse/obj.tab.o:
	+$(MAKE) -C parse

clean:
	rm -f subd
