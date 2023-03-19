all: encoder

encoder: encoder.o
	gcc -g -m32 -Wall encoder.o -o encoder

encoder.o: encoder.c
	gcc -g -m32 -Wall -c encoder.c -o encoder.o

clean:
	rm -f encoder.o
