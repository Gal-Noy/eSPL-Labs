all: menu_map

menu_map: menu_map.o
	gcc -g -m32 -Wall menu_map.o -o menu_map

menu_map.o: menu_map.c
	gcc -g -m32 -Wall -c menu_map.c

clean:
	rm -f *.o
