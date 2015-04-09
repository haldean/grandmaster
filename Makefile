# Makefile for grandmaster project
# Copyright (C) 2015, Haldean Brown
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

COPTS := -std=c99 -pedantic -Werror -Wall -Wextra -Iinclude -ggdb -O1 -DDEBUG \
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

build/pgn_parser: $(STATICLIB) test/pgn_parser.c
	$(CC) $(COPTS) test/pgn_parser.c $(LDOPTS) -Lbuild -lgrandmaster -o $@

test: test/test_rules.py build/move_parser
	python test/test_rules.py

clean:
	rm -rf build

.PHONY: objects clean move_parser test
