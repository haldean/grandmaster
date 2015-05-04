/*
 * pgn_parser.c: parser tool for the PGN data interchange format
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

#include <grandmaster/core.h>
#include <grandmaster/internal.h>
#include <gameio.h>
#include <jansson.h>
#include <stdio.h>

#define MAX_PGN_LEN 4096

int
main()
{
    struct move *last;
    size_t size;
    char pgn[MAX_PGN_LEN];
    json_t *result;

    size = fread(pgn, 1, MAX_PGN_LEN, stdin);
    if (!feof(stdin)) {
        printf("max input file size is 4K.\n");
        return 1;
    }

    last = parse_pgn(pgn, size);
    if (last == NULL) {
        printf("failed to parse PGN.\n");
        return 1;
    }

    result = move_to_json(last);
    json_dumpf(result, stdout, JSON_PRESERVE_ORDER | JSON_INDENT(4));
    printf("\n");

    /* dealloc at end of process so we can run things through valgrind, even
     * though it's otherwise unnecessary */
    json_decref(result);
    free_move_tree(last);

    return 0;
}
