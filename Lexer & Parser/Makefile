all: djcode

djcode: parser.tab.o lex.yy.o main.o
	cc -o djcode parser.tab.o lex.yy.o main.o -lm

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

main.o: main.c
	cc -c main.c

clean:
	rm -f *.o *.tab.* lex.yy.c djcode