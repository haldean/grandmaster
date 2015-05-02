/*
 * tree.c: game-tree-related functions
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
#include "grandmaster_internal.h"

#include <stdlib.h>
#include <string.h>

void
get_root(struct move *out)
{
    memset(out, 0, sizeof(struct move));
    out->player = BLACK;
    out->post_board = calloc(1, sizeof(struct board));
    load_default_board(out->post_board);
    build_access_map(out, out->post_board->access_map);
}

void
free_move_tree(struct move *move)
{
    if (move->parent != NULL)
        free_move_tree(move->parent);
    free_move(move);
}

void
free_move(struct move *move)
{
    if (move->post_board != NULL) {
        if (move->post_board->access_map != NULL)
            free(move->post_board->access_map);
        free(move->post_board);
    }
    if (move->algebraic != NULL)
        free(move->algebraic);
    free(move);
}
