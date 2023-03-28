all: count-words

count-words: count-words.o
	gcc -g -m32 -Wall count-words.o -o count-words

count-words.o: count-words.c
	gcc -g -m32 -Wall -c count-words.c -o count-words.o

clean:
	rm -f *.o
