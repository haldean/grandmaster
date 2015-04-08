CC := gcc
AR := ar
COPTS := -ansi -std=c99 -pedantic -Wall -Wextra -Iinclude -Os \
	$(shell pkg-config --cflags jansson)
LDOPTS := $(shell pkg-config --libs jansson)
HEADERS := $(wildcard src/*.h)
STATICLIB := build/libgrandmaster.a

objectfiles := $(patsubst src/%.c,build/%.o,$(wildcard src/*.c))

$(STATICLIB): $(objectfiles)
	$(AR) rvcs $@ $^

build/%.o: src/%.c
	@mkdir -p build
	@# -MD builds makefiles with dependencies in-line with the object files. We
	@# include them in the -include directive below
	$(CC) -MD -c $(COPTS) -o $@ $<

-include $(patsubst build/%.o,build/%.d,$(objectfiles))

build/move_parser: $(STATICLIB) test/move_parser.c
	$(CC) $(COPTS) test/move_parser.c $(LDOPTS) -Lbuild -lgrandmaster -o $@

move_parser: build/move_parser

clean:
	rm -rf build

.PHONY: objects clean move_parser
