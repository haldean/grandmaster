/*
 * grandmaster.h: API for grandmaster
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

#ifndef __GRANDMASTER_H__
#define __GRANDMASTER_H__

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t player_id_t;

typedef enum {
    PAWN = 'p',
    ROOK = 'R',
    KNIGHT = 'N',
    BISHOP = 'B',
    QUEEN = 'Q',
    KING = 'K'
} piece_type_t;

#define ALL_PIECES ((piece_type_t[]) { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING })

typedef enum {
    WHITE = 'w',
    BLACK = 'b'
} color_t;

#define opposite(x) ((x) == WHITE ? BLACK : WHITE)

typedef enum {
    WHITE_KINGSIDE = 0x01,
    WHITE_QUEENSIDE = 0x02,
    BLACK_KINGSIDE = 0x04,
    BLACK_QUEENSIDE = 0x08
} castles_t;

struct piece {
    piece_type_t piece_type;
    color_t color;
};

struct position {
    int8_t rank;
    int8_t file;
};

struct move {
    struct position start;
    struct position end;
    color_t player;
    char *algebraic;
    struct move *parent;
    struct board *post_board;
};

#define NO_PASSANT (-1)

struct board {
    struct piece board[8][8];
    uint8_t available_castles;
    int8_t passant_file;
};

struct game {
    player_id_t player_white;
    player_id_t player_black;
    struct move *current;
};

/* Returns a move that contains the root of the full game tree. */
void
get_root(struct move *out);

/* Parse algebraic notation and return the result. **out is set to null if the
 * input was not a valid move. */
void
parse_algebraic(
    const char *notation,
    struct move *last_move,
    struct move **out);

/* Returns true if the movement in the move struct represents a valid movement
 * for the piece that moved. This only checks the "geometry" of the move, and
 * that there was no pieces between the start and end point (if applicable). It
 * does not check for other properties, like not moving into check. */
bool
is_movement_valid(const struct move *move);

/* Returns true if the player to move is in checkmate. */
bool
in_checkmate(struct move *move);

/* Returns true if the player to move is in check or checkmate. */
bool
in_check(struct move *move);

/* Finds a piece of the given color and type that has access to move->end,
 * respecting any preexisting values in move->start. Populates the value at
 * move->start with results if any are found. If none are found, move is
 * entirely unchanged. */
void
find_piece_with_access(struct piece piece, struct move *move);

/* Returns true if the given player can attack the given square. */
bool
can_attack(struct move *move, struct position position, color_t to_move);

/* Free a move struct, leaving its parent move untouched. */
void
free_move(struct move *move);

/* Free a move struct and all of its parents. */
void
free_move_tree(struct move *move);

/* Returns true if the two boards are equivalent. */
bool
boards_equal(struct board *, struct board *);

#endif
