all: virusDetector

virusDetector: virusDetector.o
	gcc -g -m32 -Wall virusDetector.o -o virusDetector

virusDetector.o: virusDetector.c
	gcc -g -m32 -Wall -c virusDetector.c
clean:
	rm -f *.o