all: start

start: start.o
	gcc -m32 -g start.o -o start

start.o: start.s
	nasm -f elf32 start.s -o start.o

.PHONY: clean

clean:
	rm -f *.o start