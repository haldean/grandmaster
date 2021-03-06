/*
 * movement.c: encoding of chess piece movement abilities
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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define sign(x) (((x) > 0) - ((x) < 0))

#ifdef DEBUG
#  define move_fail(...) do {\
        printf("move_fail: "); \
        printf(__VA_ARGS__); \
        printf("\n"); \
        return false; } while (0);
#else
#  define move_fail(...) do { return false; } while (0);
#endif


bool
any_between(
    const struct position start,
    const struct position end,
    const struct board *board)
{
    int rank_step;
    int file_step;
    int d_rank;
    int d_file;
    const struct piece *p;

    assert((start.rank == end.rank)
        || (start.file == end.file)
        || (abs(start.rank - end.rank) == abs(start.file - end.file)));

    rank_step = sign(end.rank - start.rank);
    file_step = sign(end.file - start.file);
    d_rank = rank_step;
    d_file = file_step;

    while (start.rank + d_rank != end.rank || start.file + d_file != end.file) {
        p = &board->board[start.rank + d_rank][start.file + d_file];
        if (p->piece_type != 0)
            return true;
        d_rank += rank_step;
        d_file += file_step;
    }
    return false;
}

bool
pawn_movement_valid(const struct move *move)
{
    bool is_capture;
    struct board *b;
    struct piece *last_move_piece;
    int d_rank;
    int8_t passant_file;
    bool may_be_passant;

    b = move->parent->post_board;
    is_capture = b->board[move->end.rank][move->end.file].piece_type != 0;
    passant_file = move->parent->post_board->passant_file;
    may_be_passant =
        passant_file != NO_PASSANT
        && move->end.file == passant_file
        && move->end.file != move->start.file;

    if (is_capture) {
        if (abs(move->start.file - move->end.file) != 1)
            return false;
        d_rank = move->end.rank - move->start.rank;
        if (move->player == WHITE) {
            if (may_be_passant) {
                if (d_rank != 1 && d_rank != 2)
                    return false;
            } else {
                if (d_rank != 1)
                    return false;
            }
        } else {
            if (may_be_passant) {
                if (d_rank != -1 && d_rank != -2)
                    return false;
            } else {
                if (d_rank != -1)
                    return false;
            }
        }
    } else if (move->start.file != move->end.file) {
        /* this might be en passant. let's check it out. */
        /* we can only capture pawns that moved last turn. */
        if (move->end.file != move->parent->end.file)
            return false;
        last_move_piece = &b->board[move->parent->end.rank][move->parent->end.file];
        if (last_move_piece->piece_type != PAWN)
            return false;
        if (abs(move->start.file - move->end.file) != 1)
            return false;
        if (move->player == WHITE) {
            if (move->start.rank != 4 || move->end.rank != 5)
                return false;
        } else {
            if (move->start.rank != 3 || move->end.rank != 2)
                return false;
        }
    } else {
        d_rank = move->end.rank - move->start.rank;
        if (abs(d_rank) > 2 || d_rank == 0)
            return false;
        if (move->player == WHITE) {
            if (d_rank < 0)
                return false;
            if (d_rank == 2 && move->start.rank != 1)
                return false;
        }
        if (move->player == BLACK) {
            if (d_rank > 0)
                return false;
            if (d_rank == -2 && move->start.rank != 6)
                return false;
        }
    }

    if (!may_be_passant
            && any_between(move->start, move->end, move->parent->post_board)) {
        return false;
    }
    return true;
}

bool
rook_movement_valid(const struct move *move)
{
    int rank_constant;
    int file_constant;

    rank_constant = (move->start.rank == move->end.rank);
    file_constant = (move->start.file == move->end.file);
    if (!(rank_constant ^ file_constant))
        return false;
    return !any_between(move->start, move->end, move->parent->post_board);
}

bool
knight_movement_valid(const struct move *move)
{
    int d_rank;
    int d_file;

    d_rank = abs(move->start.rank - move->end.rank);
    d_file = abs(move->start.file - move->end.file);
    return (d_rank == 2 && d_file == 1) || (d_rank == 1 && d_file == 2);
}

bool
bishop_movement_valid(const struct move *move)
{
    int d_rank;
    int d_file;

    d_rank = abs(move->start.rank - move->end.rank);
    d_file = abs(move->start.file - move->end.file);
    if (d_rank != d_file)
        return false;
    if (any_between(move->start, move->end, move->parent->post_board))
        return false;
    return true;
}

bool
queen_movement_valid(const struct move *move)
{
    return bishop_movement_valid(move) || rook_movement_valid(move);
}

bool
castle_movement_valid(const struct move *move)
{
    castles_t castle_type;
    struct position king;
    struct position king_end;
    struct position rook;
    int file_step;

    if (move->algebraic == NULL)
        return false;

    castle_type = 0;
    if (!strncmp(move->algebraic, "0-0-0", 5) ||
            !strncmp(move->algebraic, "O-O-O", 5)) {
        castle_type = move->player == WHITE ? WHITE_QUEENSIDE : BLACK_QUEENSIDE;
    }
    else if (!strncmp(move->algebraic, "0-0", 3) ||
            !strncmp(move->algebraic, "O-O", 3)) {
        castle_type = move->player == WHITE ? WHITE_KINGSIDE : BLACK_KINGSIDE;
    }
    if (castle_type == 0)
        move_fail("castle type doesn't match consts");

    if (!(move->parent->post_board->available_castles & castle_type))
        move_fail("castle type not available");

    king.file = 4;
    if (castle_type & (WHITE_KINGSIDE | WHITE_QUEENSIDE)) {
        king.rank = 0;
        king_end.rank = 0;
        rook.rank = 0;
    } else {
        king.rank = 7;
        king_end.rank = 7;
        rook.rank = 7;
    }
    if (castle_type & (WHITE_KINGSIDE | BLACK_KINGSIDE)) {
        king_end.file = 6;
        rook.file = 7;
    } else {
        king_end.file = 2;
        rook.file = 0;
    }
    if (any_between(king, rook, move->parent->post_board))
        return false;

    file_step = sign(king_end.file - king.file);
    for (king.file += file_step;
            king.file != king_end.file;
            king.file += file_step) {
        if (can_attack(move->parent, king, move->parent->player))
            return false;
    }

    return true;
}

bool
king_movement_valid(const struct move *move)
{
    int d_rank;
    int d_file;

    d_rank = abs(move->start.rank - move->end.rank);
    d_file = abs(move->start.file - move->end.file);
    return (d_rank <= 1 && d_file <= 1) || castle_movement_valid(move);
}

bool
is_movement_valid(struct move *move)
{
    struct piece *piece;
    struct piece *captured;

    if (0 > move->start.rank || 7 < move->start.rank)
        return false;
    if (0 > move->start.file || 7 < move->start.file)
        return false;
    if (0 > move->end.rank || 7 < move->end.rank)
        return false;
    if (0 > move->end.file || 7 < move->end.file)
        return false;

    piece = &move->parent->post_board->board[move->start.rank][move->start.file];
    if (piece->piece_type == 0 || piece->color == 0)
        return false;

    /* can't capture our own pieces. */
    captured = &move->parent->post_board->board[move->end.rank][move->end.file];
    if (captured->color == piece->color)
        return false;

    if (captured->piece_type != KING && in_check(move, move->player))
        return false;

    switch (piece->piece_type) {
        case PAWN:
            return pawn_movement_valid(move);
        case ROOK:
            return rook_movement_valid(move);
        case KNIGHT:
            return knight_movement_valid(move);
        case BISHOP:
            return bishop_movement_valid(move);
        case QUEEN:
            return queen_movement_valid(move);
        case KING:
            return king_movement_valid(move);
    }
    return false;
}
