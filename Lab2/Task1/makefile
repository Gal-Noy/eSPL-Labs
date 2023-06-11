all: addresses

addresses: addresses.o
	gcc -g -m32 -Wall addresses.o -o addresses

addresses.o: addresses.c
	gcc -g -m32 -Wall -c addresses.c
clean:
	rm -f *.o
