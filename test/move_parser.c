/*
 * move_parser.c: test algebraic notation parsing
 * Copyright (C) 2015, Haldean Brown
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "grandmaster/core.h"
#include "grandmaster/internal.h"

#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    struct move *last;
    struct move *next;
    int i;
    char *notation;
    json_t *results;

    if (argc <= 1) {
        printf("FAILED: missing FEN starting position\n");
        return 1;
    }

    results = json_array();

    if (strlen(argv[1])) {
        last = parse_fen(argv[1], strlen(argv[1]));
    } else {
        last = calloc(1, sizeof(struct move));
        get_root(last);
    }

    for (i = 2; i < argc; i++) {
        notation = argv[i];
        parse_algebraic(notation, last, &next);
        if (next == NULL) {
            printf("FAILED: parse_algebraic returned error on %s\n", notation);
            return 1;
        }
        json_array_append_new(results, move_to_json(next));
        last = next;
    }

    json_dumpf(results, stdout, JSON_PRESERVE_ORDER | JSON_INDENT(2));
    printf("\n");
    return 0;
}
