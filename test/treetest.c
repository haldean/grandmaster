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
#include "grandmaster/tree.h"

#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    struct game_tree *gt;
    json_t *results;
    game_id_t g1, g2, g3;
    bool quiet;

    quiet = argc > 1 && strncmp(argv[1], "quiet", 6) == 0;

    gt = calloc(1, sizeof(struct game_tree));
    init_gametree(gt);

    g1 = new_game(gt, 12, 56);
    g2 = new_game(gt, 34, 56);
    g3 = new_game(gt, 12, 34);

    make_move(gt, g1, "e4");
    make_move(gt, g1, "d5");
    make_move(gt, g2, "e4");
    make_move(gt, g3, "Nc3");

    if (gt->n_states != 4) {
        fprintf(stderr, "expected 4 states, got %lu\n", gt->n_states);
        return 1;
    }
    if (gt->games[g1]->current->parent != gt->games[g2]->current) {
        fprintf(stderr, "g1's parent should be g2\n");
        return 1;
    }

    if (quiet) {
        fprintf(stderr, "OK\n");
    } else {
        results = game_tree_to_json(gt);
        json_dumpf(results, stdout, JSON_PRESERVE_ORDER | JSON_INDENT(2));
        printf("\n");
    }

    free_game_tree(gt);
    return 0;
}
