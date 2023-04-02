all: myshell looper

myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall myshell.o LineParser.o -o myshell

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall -c LineParser.c

myshell.o: myshell.c LineParser.h
	gcc -g -m32 -Wall -c myshell.c

clean:
	rm -f *.o myshell