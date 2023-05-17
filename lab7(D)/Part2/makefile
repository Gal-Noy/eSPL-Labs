all: start

start: start.o main.o
	gcc -m32 -g start.o main.o -o start

start.o: start.s
	nasm -f elf32 start.s -o start.o

main.o: main.s
	nasm -f elf32 main.s -o main.o

.PHONY: clean

clean:
	rm -f *.o start