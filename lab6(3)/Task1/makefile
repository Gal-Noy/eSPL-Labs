all: start

start: start.o util.o
	ld -m elf_i386 start.o util.o -o start

util.o:
	gcc -m32 -g -c util.c -o util.o
	
start.o:
	nasm -f elf32 start.s -o start.o

.PHONY: clean

clean:
	rm -f *.o start