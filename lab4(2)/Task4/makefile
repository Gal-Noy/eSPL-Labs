myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall myshell.o LineParser.o -o myshell

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall -c LineParser.c

myshell.o: myshell.c LineParser.h
	gcc -g -m32 -Wall -c myshell.c

mypipe: mypipe.o
	gcc -g -m32 -Wall mypipe.o -o mypipe

mypipe.o: mypipe.c
	gcc -g -m32 -Wall -c mypipe.c

clean:
	rm -f *.o myshell mypipe