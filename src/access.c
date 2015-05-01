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
#include "grandmaster_internal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define sign(x) (((x) > 0) - ((x) < 0))

void
find_all_with_access(
    struct piece piece,
    struct move *move,
    int *n_results,
    struct position **results)
{
    int8_t rank;
    int8_t file;
    struct move test_move;
    struct piece *board_piece;
    struct position *res;
    const int max_res = 16;
    const int max_res_arraylen = max_res * sizeof(struct position);

    *n_results = 0;
    /* instead of repeatedly growing the result array, alloc an array large
     * enough to store the maximum number of responses and then realloc at the
     * end of the function down to the appropriate size. Even at max length,
     * this is only 32 bytes of data. */
    res = malloc(max_res_arraylen);
    if (res == NULL) {
        return;
    }
    memset(res, 0xFF, max_res_arraylen);

    memset(&test_move, 0x00, sizeof(struct move));
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
            if (board_piece->piece_type == 0)
                continue;
            if (piece.piece_type != 0 &&
                    board_piece->piece_type != piece.piece_type)
                continue;
            if (piece.color != 0 && board_piece->color != piece.color)
                continue;
            test_move.start.rank = rank;
            test_move.start.file = file;
            test_move.player = board_piece->color;
            apply_movement(&test_move);
            if (is_movement_valid(&test_move) && *n_results < max_res) {
                res[*n_results] = test_move.start;
                (*n_results)++;
            }
        }
    }

    *results = realloc(res, *n_results * sizeof(struct position));
    if (*results == NULL) {
        *n_results = 0;
        free(res);
        return;
    }
}

void
find_piece_with_access(struct piece piece, struct move *move)
{
    int8_t rank;
    int8_t file;
    struct move test_move;
    struct piece *board_piece;

    memset(&test_move, 0x00, sizeof(struct move));
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
            if (board_piece->piece_type == 0)
                continue;
            if (piece.piece_type != 0 &&
                    board_piece->piece_type != piece.piece_type)
                continue;
            if (piece.color != 0 && board_piece->color != piece.color)
                continue;
            test_move.start.rank = rank;
            test_move.start.file = file;
            test_move.player = board_piece->color;
            apply_movement(&test_move);
            if (is_movement_valid(&test_move)) {
                move->start = test_move.start;
                free(test_move.post_board);
                return;
            }
            free(test_move.post_board);
            test_move.post_board = NULL;
        }
    }
}

bool
can_attack(
    struct move *move,
    struct position position,
    color_t to_move
    )
{
    struct move test_move;
    struct piece constraint;

    test_move.parent = move;
    test_move.start.rank = -1;
    test_move.start.file = -1;
    test_move.end.rank = position.rank;
    test_move.end.file = position.file;

    constraint.color = to_move;
    constraint.piece_type = 0;

    find_piece_with_access(constraint, &test_move);

    return test_move.start.rank != -1;
}

bool
can_block(
    struct move *move,
    struct position mover,
    struct position target,
    color_t to_move
    )
{
    struct piece *moving_piece;
    struct move test_move;
    struct piece constraint;
    int rank_step;
    int file_step;
    int d_rank;
    int d_file;

    moving_piece = &move->post_board->board[mover.rank][mover.file];
    if (moving_piece->piece_type == KNIGHT) {
        /* No blocking a knight. You're done for. */
        return false;
    }

    rank_step = sign(target.rank - mover.rank);
    file_step = sign(target.file - mover.file);
    d_rank = rank_step;
    d_file = file_step;

    test_move.parent = move;
    test_move.start.rank = -1;
    test_move.start.file = -1;
    constraint.color = to_move;
    constraint.piece_type = 0;

    while (mover.rank + d_rank != target.rank
            || mover.file + d_file != target.file) {
        test_move.end.rank = mover.rank + d_rank;
        test_move.end.file = mover.file + d_file;

        find_piece_with_access(constraint, &test_move);
        if (test_move.start.rank != -1) {
            return true;
        }

        d_rank += rank_step;
        d_file += file_step;
    }
    return false;
}
