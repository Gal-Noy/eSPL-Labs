all: hexeditplus

hexeditplus: hexeditplus.o
	gcc -g -m32 -Wall hexeditplus.o -o hexeditplus

hexeditplus.o: hexeditplus.c
	gcc -g -m32 -Wall -c hexeditplus.c
	
clean:
	rm -f *.o hexeditplus