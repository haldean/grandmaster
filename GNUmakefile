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

COPTS := -std=c99 -pedantic -Werror -Wall -Wextra -Iinclude -ggdb -O0 -DDEBUG \
	$(shell pkg-config --cflags jansson)
LDOPTS := $(shell pkg-config --libs jansson)
HEADERS := $(wildcard src/*.h)
STATICLIB := build/libgrandmaster.a

objectfiles := $(patsubst src/%.c,build/%.o,$(wildcard src/*.c))

$(STATICLIB): $(objectfiles)
	$(AR) rvcs $@ $^

dist: $(STATICLIB)

build/%.o: src/%.c
	@mkdir -p build
	@# -MD builds makefiles with dependencies in-line with the object files. We
	@# include them in the -include directive below
	$(CC) $(COPTS) -MD -c -o $@ $<

-include $(patsubst build/%.o,build/%.d,$(objectfiles))

testbin/%: test/%.c $(STATICLIB)
	@mkdir -p testbin
	$(CC) $(COPTS) $< -Lbuild -lgrandmaster $(LDOPTS) -o $@

test: test/test_rules.py testbin/move_parser testbin/test_rules_harness testbin/is_in_check testbin/treetest
	python test/test_rules.py -b -v $(only_test)
	testbin/treetest quiet

clean:
	rm -rf build testbin

.PHONY: dist clean move_parser test
