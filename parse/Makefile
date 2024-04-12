CC=gcc
CFLAGS=-O2 -ggdb3 -Wall
OBJ=stack.o strop.o

#parse: lex.yy.c stack.c strop.c
#	$(CC) $^ -c

obj: main.c lex.yy.c stack.c strop.c
	$(CC) $(CFLAGS) -o $@ $^

lex.yy.c: obj.l
	flex obj.l

clean:
	rm -f obj lex.yy.c *.o
