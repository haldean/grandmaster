/*
 * test_rules_harness.c: ensures that moves applied to a given board have the
 *                       intended result.
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

#include <grandmaster.h>
#include <gameio.h>
#include <stdio.h>
#include <string.h>

#define MAX_PGN_LEN 4096

int
main(int argc, char *argv[])
{
    struct move *last;
    struct move *expected;
    size_t size;
    bool match;
    char pgn[MAX_PGN_LEN];

    if (argc < 3) {
        fprintf(stderr, "no FEN data given on command line\n");
        return 1;
    }
    expected = parse_fen(argv[2], strlen(argv[2]));
    if (expected == NULL) {
        fprintf(stderr, "failed to parse FEN string %s\n", argv[2]);
        return 1;
    }

    last = parse_fen(argv[1], strlen(argv[1]));
    if (last == NULL) {
        fprintf(stderr, "failed to parse FEN string %s\n", argv[1]);
        return 1;
    }

    size = fread(pgn, 1, MAX_PGN_LEN, stdin);
    if (!feof(stdin)) {
        fprintf(stderr, "max input file size is 4K.\n");
        return 1;
    }

    last = parse_pgn(pgn, size, last);
    if (last == NULL) {
        fprintf(stderr, "failed to parse PGN.\n");
        return 1;
    }

    match = boards_equal(last->post_board, expected->post_board);
    if (!match) {
        fprintf(stderr, "expected %s\n", move_to_fen(expected));
        fprintf(stderr, "actual   %s\n", move_to_fen(last));
        fprintf(stderr, "FAIL\n");
        return 2;
    }
    fprintf(stderr, "OK\n");

    /* dealloc at end of process so we can run things through valgrind, even
     * though it's otherwise unnecessary */
    free_move_tree(last);
    free_move_tree(expected);

    return 0;
}
