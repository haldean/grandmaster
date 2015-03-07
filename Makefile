CC=gcc
COPTS=-ansi -std=c99 -pedantic -Wall -Wextra -g
CSRC=algebraic.c gameio.c tree.c movement.c access.c kings.c
STATICLIB=libgrandmaster.a

$(STATICLIB): $(CSRC) *.h
	$(CC) -c $(COPTS) -c $(CSRC)
	$(AR) rvcs $(STATICLIB) *.o

test: $(STATICLIB)
	$(CC) $(COPTS) test_algebraic.c -L $(PWD) -lgrandmaster -o test_algebraic

clean:
	rm -f *.o $(STATICLIB)
