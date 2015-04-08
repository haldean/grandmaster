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

build/move_parser: $(STATICLIB) test/move_parser.c
	$(CC) $(COPTS) test/move_parser.c -L build -lgrandmaster -o $@

move_parser: build/move_parser

clean:
	rm -rf build

.PHONY: objects clean move_parser
