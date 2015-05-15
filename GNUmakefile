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

COPTS := -std=c99 -pedantic -Werror -Wall -Wextra -Iinclude -ggdb -O0 \
	$(shell pkg-config --cflags jansson) -D_GNU_SOURCE
LDOPTS := $(shell pkg-config --libs jansson) -lm
HEADERS := $(wildcard src/*.h)
STATICLIB := dist/libgrandmaster.a

libobjects := $(patsubst src/%.c,build/lib/%.o,$(wildcard src/*.c))
gmobjects := $(patsubst gm/%.c,build/gm/%.o,$(wildcard gm/*.c))

$(STATICLIB): $(libobjects)
	@mkdir -p dist
	$(AR) rvcs $@ $^

build/lib/%.o: src/%.c
	@mkdir -p build/lib
	@# -MD builds makefiles with dependencies in-line with the object files. We
	@# include them in the -include directive below
	$(CC) $(COPTS) -MD -c -o $@ $<

build/gm/%.o: gm/%.c
	@mkdir -p build/gm
	$(CC) $(COPTS) -MD -c -o $@ $<

-include $(patsubst build/lib/%.o,build/lib/%.d,$(libobjects))
-include $(patsubst build/gm/%.o,build/gm/%.d,$(gmobjects))

dist/gm: $(STATICLIB) $(gmobjects)
	@mkdir -p dist
	$(CC) $(LDOPTS) $(gmobjects) -Ldist -lgrandmaster -o $@

testbin/%: test/%.c $(STATICLIB)
	@mkdir -p testbin
	$(CC) $(COPTS) $< -Ldist -lgrandmaster $(LDOPTS) -o $@

test: test/test_rules.py testbin/move_parser testbin/test_rules_harness testbin/is_in_check testbin/treetest
	python test/test_rules.py -b -v $(only_test)
	testbin/treetest quiet

checkbin/%: check/%.c $(STATICLIB)
	@mkdir -p checkbin
	$(CC) $(COPTS) $< -Ldist -lgrandmaster -lcheck $(LDOPTS) -o $@

check: checkbin/test_core
	checkbin/test_core

clean:
	rm -rf build testbin checkbin dist

.PHONY: clean move_parser test check
