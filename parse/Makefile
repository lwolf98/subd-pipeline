CC=gcc
CFLAGS=-O0 -ggdb3 -Wall
OBJ=main.o stack.o strop.o obj.tab.o lex.yy.o

obj: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

obj.tab.c obj.tab.h: obj.y
	bison --defines -Wcounterexamples obj.y

obj.tab.o: obj.tab.c obj.tab.h

lex.yy.c: obj.l
	flex obj.l

clean:
	rm -f obj lex.yy.* obj.tab.* *.o
