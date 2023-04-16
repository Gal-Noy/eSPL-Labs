all: myshell looper

myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall myshell.o LineParser.o -o myshell

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall -c LineParser.c

myshell.o: myshell.c LineParser.h
	gcc -g -m32 -Wall -c myshell.c

looper: looper.o
	gcc -g -m32 -Wall looper.c -o looper

looper.o: looper.c
	gcc -g -m32 -Wall -c looper.c -o looper.o

.PHONY: clean

clean:
	rm -f *.o looper myshell