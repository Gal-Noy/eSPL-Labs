all: base

base: base.o
	gcc -g -m32 -Wall base.o -o base

base.o: base.c
	gcc -g -m32 -Wall -c base.c

clean:
	rm -f *.o
