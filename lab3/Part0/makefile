all: bubblesort hexaPrint

bubblesort: bubblesort.o
	gcc -g -m32 -Wall bubblesort.o -o bubblesort

bubblesort.o: bubblesort.c
	gcc -g -m32 -Wall -c bubblesort.c -o bubblesort.o

hexaPrint: hexaPrint.o
	gcc -g -m32 -Wall hexaPrint.o -o hexaPrint

hexaPrint.o: hexaPrint.c
	gcc -g -m32 -Wall -c hexaPrint.c -o hexaPrint.o

clean:
	rm -f *.o