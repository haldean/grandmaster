/*
 * access.c: piece access queries
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

void
find_piece_with_access(struct piece piece, struct move *move)
{
    int8_t rank;
    int8_t file;
    struct move test_move;
    struct piece *board_piece;

    test_move.parent = move->parent;
    test_move.end = move->end;

    for (rank = 0; rank < 8; rank++) {
        /* we could do this more efficiently by skipping the loop altogether in
         * this case, but I like the succinctness of doing this all in one loop
         * with no special cases. */
        if (move->start.rank != -1 && move->start.rank != rank)
            continue;
        for (file = 0; file < 8; file++) {
            if (move->start.file != -1 && move->start.file != file)
                continue;
            board_piece = &move->parent->post_board->board[rank][file];
            if (piece.piece_type != 0 &&
                    board_piece->piece_type != piece.piece_type)
                continue;
            if (piece.color != 0 && board_piece->color != piece.color)
                continue;
            test_move.start.rank = rank;
            test_move.start.file = file;
            if (is_movement_valid(&test_move)) {
                move->start = test_move.start;
                return;
            }
        }
    }
}

