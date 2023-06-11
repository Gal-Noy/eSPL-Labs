all: mypipeline

mypipeline: mypipeline.o
	gcc -g -m32 -Wall mypipeline.o -o mypipeline

mypipeline.o: mypipeline.c
	gcc -g -m32 -Wall -c mypipeline.c

.PHONY: clean

clean:
	rm -f *.o mypipeline