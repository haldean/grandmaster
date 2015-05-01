/*
 * board.c: manipulation of board objects
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void
apply_movement(struct move *m)
{
    assert(m->parent->post_board != NULL);
    if (m->post_board == NULL) {
        m->post_board = calloc(1, sizeof(struct board));
        memcpy(m->post_board, m->parent->post_board, sizeof(struct board));
        m->post_board->board[m->end.rank][m->end.file] =
            m->post_board->board[m->start.rank][m->start.file];
        m->post_board->board[m->start.rank][m->start.file] =
            (struct piece) { .color = 0, .piece_type = 0 };
    }
}
