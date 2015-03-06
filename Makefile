CC=gcc
COPTS=-ansi -pedantic -Wall -O3

release: *.c *.h
	$(CC) $(COPTS) *.c -c
