all: myELF

myELF: myELF.o
	gcc -g -m32 -Wall myELF.o -o myELF

myELF.o: myELF.c
	gcc -g -m32 -Wall -c myELF.c
clean:
	rm -f myELF.o myELF