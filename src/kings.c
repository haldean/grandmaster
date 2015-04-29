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
#include <stdio.h>

// Find the king of the player whose turn it is.
bool
find_king(
        struct move *move,
        struct position *king_position,
        color_t player)
{
    struct piece *p;
    uint8_t king_rank;
    uint8_t king_file;
    bool found_king;

    found_king = false;
    for (king_rank = 0; king_rank < 8; king_rank++) {
        for (king_file = 0; king_file < 8; king_file++) {
            p = &move->post_board->board[king_rank][king_file];
            if (p->color != player)
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
in_check(struct move *move, color_t player)
{
    struct position king_position;
    bool found_king;

    found_king = find_king(move, &king_position, player);
    assert(found_king);

    return can_attack(move, king_position, opposite(player));
}

bool
in_checkmate(struct move *move, color_t player)
{
    struct position king_position;
    struct position test_pos;
    struct piece constraints;
    struct move *test_move;
    struct position *threats;
    struct board *b;
    char *cap_str;
    char move_str[6];
    int d_file;
    int d_rank;
    int n_threats;
    bool found_king;

    b = move->post_board;
    found_king = find_king(move, &king_position, player);
    assert(found_king);

    if (!can_attack(move, king_position, opposite(player)))
        /* We're not even in check, let alone checkmate. */
        return false;

    /* See if the king can escape by moving out of check or capturing the piece
     * itself. */
    for (d_file = -1; d_file <= 1; d_file++) {
        for (d_rank = -1; d_rank <= 1; d_rank++) {
            if (d_file == 0 && d_rank == 0)
                continue;
            test_pos.rank = king_position.rank + d_rank;
            test_pos.file = king_position.file + d_file;
            if (0 > test_pos.rank || test_pos.rank > 7)
                continue;
            if (0 > test_pos.file || test_pos.file > 7)
                continue;
            if (b->board[test_pos.rank][test_pos.file].color == player)
                continue;
            if (b->board[test_pos.rank][test_pos.file].piece_type == 0)
                cap_str = "";
            else
                cap_str = "x";

            snprintf(
                move_str, 6,
                "K%s%c%c",
                cap_str,
                test_pos.file + 'a',
                test_pos.rank + '1');
            parse_algebraic(move_str, move, &test_move);
            if (test_move != NULL) {
                printf("can escape with %s\n", move_str);
                free_move(test_move);
                return false;
            }
        }
    }

    /* If not, let's see if something else can block whatever is threatening the
     * king. */
    constraints.color = opposite(player);
    constraints.piece_type = -1;
    find_all_with_access(constraints, move, &n_threats, &threats);

    /* If more than one threat exists, double check! We're hosed. */
    if (n_threats > 1)
        return true;

    return true;
}
