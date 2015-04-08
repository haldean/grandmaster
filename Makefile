CC := gcc
AR := ar
COPTS := -ansi -std=c99 -pedantic -Wall -Wextra -g -Isrc
HEADERS := $(wildcard src/*.h)
STATICLIB := build/libgrandmaster.a

csrcs := $(wildcard src/*.c)
objectfiles := $(patsubst src/%.c,build/%.o,$(csrcs))

$(STATICLIB): $(objectfiles)
	$(AR) rvcs $@ $^

objects: $(objectfiles)

build/%.o: src/%.c
	@[ -d build ] || mkdir build
	$(CC) -c $(COPTS) -o $@ $<

build/test_algebraic: $(STATICLIB) $(wildcard test/*.c)
	$(CC) $(COPTS) test/*.c -L build -lgrandmaster -o $@

test: build/test_algebraic
	build/test_algebraic

clean:
	rm -f build

.PHONY: objects clean test
