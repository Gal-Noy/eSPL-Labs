all: hexeditplus digit_count

hexeditplus: task1.o
	gcc -g -m32 -Wall task1.o -o hexeditplus

task1.o: task1.c
	gcc -g -m32 -Wall -c task1.c

digit_count: task4.o
	gcc -g -m32 -Wall task4.o -o digit_count

task4.o: task4.c
	gcc -g -m32 -Wall -fno-pie -fno-stack-protector -c task4.c

.PHONY: clean

clean:
	rm -f *.o hexeditplus digit_count