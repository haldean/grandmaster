/*
 * pgn_verifier.c: ensures that grandmaster correctly interprets PGN notation
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
#include <jansson.h>
#include <stdio.h>
#include <string.h>

#define MAX_PGN_LEN 4096

int
main(int argc, char *argv[])
{
    struct move *last;
    struct move *expected;
    struct json_t *last_json;
    struct json_t *expected_json;
    size_t size;
    bool match;
    char pgn[MAX_PGN_LEN];

    if (argc < 2) {
        fprintf(stderr, "no FEN data given on command line\n");
        return 1;
    }
    expected = parse_fen(argv[1], strlen(argv[1]));
    if (expected == NULL) {
        fprintf(stderr, "failed to parse FEN string %s\n", argv[1]);
        return 1;
    }

    size = fread(pgn, 1, MAX_PGN_LEN, stdin);
    if (!feof(stdin)) {
        fprintf(stderr, "max input file size is 4K.\n");
        return 1;
    }

    last = parse_pgn(pgn, size);
    if (last == NULL) {
        fprintf(stderr, "failed to parse PGN.\n");
        return 1;
    }

    match = boards_equal(last->post_board, expected->post_board);
    if (!match) {
        last_json = board_to_json(last->post_board);
        expected_json = board_to_json(expected->post_board);
        json_dumpf(last_json, stdout, JSON_PRESERVE_ORDER | JSON_INDENT(4));
        json_dumpf(expected_json, stdout, JSON_PRESERVE_ORDER | JSON_INDENT(4));
        fprintf(stderr, "FAIL\n");
    } else
        fprintf(stderr, "OK\n");

    /* dealloc at end of process so we can run things through valgrind, even
     * though it's otherwise unnecessary */
    free_move_tree(last);
    free_move_tree(expected);

    return match ? 0 : 1;
}
