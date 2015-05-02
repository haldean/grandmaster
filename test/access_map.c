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
    struct move *m;
    struct access_map *map;
    int rank;
    int file;
    int i;
    int n;

    if (argc < 2) {
        fprintf(stderr, "no FEN data given on command line\n");
        return 1;
    }

    m = parse_fen(argv[1], strlen(argv[1]));
    if (m == NULL) {
        fprintf(stderr, "failed to parse FEN string %s\n", argv[1]);
        return 1;
    }
    map = m->post_board->access_map;

    printf("{\n");
    for (rank = 0; rank < 8; rank++) {
        printf("    {\n");
        for (file = 0; file < 8; file++) {
            n = map->board[rank][file].n_accessors;
            if (n > 0) {
                printf("        {%d, (struct position[]) {\n", n);
                for (i = 0; i < n; i++) {
                    printf("            (struct position) {%d, %d},\n",
                            map->board[rank][file].accessors[i].rank,
                            map->board[rank][file].accessors[i].file);
                }
                printf("        },\n");
            } else {
                printf("        {%d, NULL},\n", n);
            }
        }
        printf("    },\n");
    }
    printf("}\n");

    free_move_tree(m);
    return 0;
}
