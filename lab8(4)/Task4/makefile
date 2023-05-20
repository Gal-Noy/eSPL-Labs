all: task1 task4

task1: task1.o
	gcc -g -m32 -Wall task1.o -o task1

task1.o: task1.c
	gcc -g -m32 -Wall -c task1.c

task4: task4.o
	gcc -g -m32 -Wall task4.o -o task4

task4.o: task4.c
	gcc -g -m32 -Wall -c task4.c

.PHONY: clean

clean:
	rm -f *.o task1 task4