all: virusDetector

virusDetector: AntiVirus.o
	gcc -g -m32 -Wall AntiVirus.o -o virusDetector

AntiVirus.o: AntiVirus.c
	gcc -g -m32 -Wall -c AntiVirus.c
clean:
	rm -f *.o