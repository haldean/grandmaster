/*
 * kings.c: tools to check for check and checkmate status
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

bool
can_attack(
    struct move *move,
    struct position king_position,
    color_t to_move
    )
{
    struct move test_move;
    struct piece constraint;

    test_move.parent = move;
    test_move.start.rank = -1;
    test_move.start.file = -1;
    test_move.end.rank = king_position.rank;
    test_move.end.file = king_position.file;

    constraint.color = to_move;
    constraint.piece_type = 0;

    find_piece_with_access(constraint, &test_move);

    return test_move.start.rank != -1;
}

// Find the king of the player whose turn it is.
bool
find_king(struct move *move, struct position *king_position)
{
    struct piece *p;
    uint8_t king_rank;
    uint8_t king_file;
    bool found_king;

    found_king = false;
    for (king_rank = 0; king_rank < 8; king_rank++) {
        for (king_file = 0; king_file < 8; king_file++) {
            p = &move->post_board->board[king_rank][king_file];
            if (p->color == move->player)
                continue;
            if (p->piece_type == KING) {
                found_king = true;
                break;
            }
        }
        if (found_king)
            break;
    }

    if (!found_king)
        return false;

    king_position->rank = king_rank;
    king_position->file = king_file;
    return true;
}

bool
in_check(struct move *move)
{
    struct position king_position;
    bool found_king;

    found_king = find_king(move, &king_position);
    assert(found_king);

    return can_attack(
        move, king_position, move->player);
}

bool
in_checkmate(struct move *move)
{
    (void)(move); /* suppress unused warning */
    // TODO: implement this
    return false;
}
