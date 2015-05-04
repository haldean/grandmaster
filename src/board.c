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

#include "grandmaster/core.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void
load_default_board(struct board *b)
{
    b->available_castles =
        WHITE_KINGSIDE | WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE;
    b->passant_file = NO_PASSANT;

    b->board[0][0] = (struct piece) { .color = WHITE, .piece_type = ROOK };
    b->board[0][1] = (struct piece) { .color = WHITE, .piece_type = KNIGHT };
    b->board[0][2] = (struct piece) { .color = WHITE, .piece_type = BISHOP };
    b->board[0][3] = (struct piece) { .color = WHITE, .piece_type = QUEEN };
    b->board[0][4] = (struct piece) { .color = WHITE, .piece_type = KING };
    b->board[0][5] = (struct piece) { .color = WHITE, .piece_type = BISHOP };
    b->board[0][6] = (struct piece) { .color = WHITE, .piece_type = KNIGHT };
    b->board[0][7] = (struct piece) { .color = WHITE, .piece_type = ROOK };
    b->board[1][0] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    b->board[1][1] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    b->board[1][2] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    b->board[1][3] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    b->board[1][4] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    b->board[1][5] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    b->board[1][6] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    b->board[1][7] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    b->board[6][0] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    b->board[6][1] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    b->board[6][2] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    b->board[6][3] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    b->board[6][4] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    b->board[6][5] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    b->board[6][6] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    b->board[6][7] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    b->board[7][0] = (struct piece) { .color = BLACK, .piece_type = ROOK };
    b->board[7][1] = (struct piece) { .color = BLACK, .piece_type = KNIGHT };
    b->board[7][2] = (struct piece) { .color = BLACK, .piece_type = BISHOP };
    b->board[7][3] = (struct piece) { .color = BLACK, .piece_type = QUEEN };
    b->board[7][4] = (struct piece) { .color = BLACK, .piece_type = KING };
    b->board[7][5] = (struct piece) { .color = BLACK, .piece_type = BISHOP };
    b->board[7][6] = (struct piece) { .color = BLACK, .piece_type = KNIGHT };
    b->board[7][7] = (struct piece) { .color = BLACK, .piece_type = ROOK };

    b->access_map = calloc(1, sizeof(struct access_map));
}

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

bool
boards_equal(struct board *b1, struct board *b2)
{
    int rank;
    int file;
    struct piece *p1;
    struct piece *p2;

    if (b1->available_castles != b2->available_castles)
        return false;
    for (rank = 0; rank < 8; rank++) {
        for (file = 0; file < 8; file++) {
            p1 = &b1->board[rank][file];
            p2 = &b2->board[rank][file];
            if (p1->color != p2->color)
                return false;
            if (p1->piece_type != p2->piece_type)
                return false;
        }
    }
    return true;
}
