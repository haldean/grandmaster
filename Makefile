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
	@mkdir -p build
	@# -MD builds makefiles with dependencies in-line with the object files. We
	@# include them in the -include directive below
	$(CC) -MD -c $(COPTS) -o $@ $<

-include $(patsubst build/%.o,build/%.d,$(objectfiles))

build/test_algebraic: $(STATICLIB) $(wildcard test/*.c)
	$(CC) $(COPTS) test/*.c -L build -lgrandmaster -o $@

test: build/test_algebraic
	build/test_algebraic

clean:
	rm -rf build

.PHONY: objects clean test
