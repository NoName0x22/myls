CC=gcc
CFLAGS=-Wall -ansi -pedantic 
PROG=myls.c

default: program

program:
	$(CC) $(CFLAGS) $(PROG) -o myls



clean: 
	rm -f *.o myls

