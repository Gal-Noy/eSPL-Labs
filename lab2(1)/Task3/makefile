all: menu

menu: menu.o
	gcc -g -m32 -Wall menu.o -o menu

menu.o: menu.c
	gcc -g -m32 -Wall -c menu.c

clean:
	rm -f *.o
