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

#include <stdlib.h>
#include <string.h>

void
get_root(struct move *out)
{
    memset(out, 0, sizeof(struct move));
    out->player = BLACK;

    out->post_board = calloc(1, sizeof(struct board));
    out->post_board->available_castles =
        WHITE_KINGSIDE | WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE;

    out->post_board->board[0][0] = (struct piece) { .color = WHITE, .piece_type = ROOK };
    out->post_board->board[0][1] = (struct piece) { .color = WHITE, .piece_type = KNIGHT };
    out->post_board->board[0][2] = (struct piece) { .color = WHITE, .piece_type = BISHOP };
    out->post_board->board[0][3] = (struct piece) { .color = WHITE, .piece_type = QUEEN };
    out->post_board->board[0][4] = (struct piece) { .color = WHITE, .piece_type = KING };
    out->post_board->board[0][5] = (struct piece) { .color = WHITE, .piece_type = BISHOP };
    out->post_board->board[0][6] = (struct piece) { .color = WHITE, .piece_type = KNIGHT };
    out->post_board->board[0][7] = (struct piece) { .color = WHITE, .piece_type = ROOK };
    out->post_board->board[1][0] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    out->post_board->board[1][1] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    out->post_board->board[1][2] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    out->post_board->board[1][3] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    out->post_board->board[1][4] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    out->post_board->board[1][5] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    out->post_board->board[1][6] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    out->post_board->board[1][7] = (struct piece) { .color = WHITE, .piece_type = PAWN };
    out->post_board->board[6][0] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    out->post_board->board[6][1] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    out->post_board->board[6][2] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    out->post_board->board[6][3] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    out->post_board->board[6][4] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    out->post_board->board[6][5] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    out->post_board->board[6][6] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    out->post_board->board[6][7] = (struct piece) { .color = BLACK, .piece_type = PAWN };
    out->post_board->board[7][0] = (struct piece) { .color = BLACK, .piece_type = ROOK };
    out->post_board->board[7][1] = (struct piece) { .color = BLACK, .piece_type = KNIGHT };
    out->post_board->board[7][2] = (struct piece) { .color = BLACK, .piece_type = BISHOP };
    out->post_board->board[7][3] = (struct piece) { .color = BLACK, .piece_type = QUEEN };
    out->post_board->board[7][4] = (struct piece) { .color = BLACK, .piece_type = KING };
    out->post_board->board[7][5] = (struct piece) { .color = BLACK, .piece_type = BISHOP };
    out->post_board->board[7][6] = (struct piece) { .color = BLACK, .piece_type = KNIGHT };
    out->post_board->board[7][7] = (struct piece) { .color = BLACK, .piece_type = ROOK };
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
