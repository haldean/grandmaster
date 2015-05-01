/*
 * map.c: build access maps for boards
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

#include "grandmaster.h"

#include <stdlib.h>

void
build_access_map(struct move *move, struct access_map *out)
{
    int rank;
    int file;
    struct piece constraints;
    struct move test_move;

    constraints.color = 0;
    constraints.piece_type = 0;

    test_move.parent = move;

    for (rank = 0; rank < 8; rank++) {
        for (file = 0; file < 8; file++) {
            test_move.start.rank = -1;
            test_move.start.file = -1;
            test_move.end.rank = rank;
            test_move.end.file = file;

            find_all_with_access(
                constraints,
                &test_move,
                &out->board[rank][file].n_accessors,
                &out->board[rank][file].accessors
            );
        }
    }
}

void
free_access_map(struct access_map *map)
{
    int rank;
    int file;
    for (rank = 0; rank < 8; rank++)
        for (file = 0; file < 8; file++)
            if (map->board[rank][file].n_accessors > 0)
                free(map->board[rank][file].accessors);
    free(map);
}
