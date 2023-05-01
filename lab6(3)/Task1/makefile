all: start

start: start.o util.o
	ld -m elf_i386 start.o util.o -o start

util.o: util.c
	gcc -m32 -Wall -g -c -nostdlib -fno-stack-protector util.c -o util.o
	
start.o: start.s
	nasm -f elf32 start.s -o start.o

.PHONY: clean

clean:
	rm -f *.o start