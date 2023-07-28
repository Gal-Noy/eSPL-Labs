all: loader

loader: loader.o
	gcc -g -m32 -Wall loader.o -o loader

loader.o: loader.c
	gcc -g -m32 -Wall -c loader.c

.PHONY: clean

clean:
	rm -f *.o loader