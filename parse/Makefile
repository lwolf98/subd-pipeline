CC=gcc
CFLAGS=-O2 -ggdb3 -Wall
OBJ=main.o stack.o strop.o obj.tab.o lex.yy.o

#parse: lex.yy.c stack.c strop.c
#	$(CC) $^ -c

obj: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

obj.tab.c obj.tab.h: obj.y
	bison --defines -Wcounterexamples obj.y

obj.tab.o: obj.tab.c obj.tab.h

lex.yy.c: obj.l
	flex obj.l

clean:
	rm -f obj lex.yy.* obj.tab.* *.o
