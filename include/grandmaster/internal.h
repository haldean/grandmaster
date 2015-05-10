/*
 * grandmaster_internal.h: private grandmaster APIs
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

#ifndef __GRANDMASTER_INTERNAL_H__
#define __GRANDMASTER_INTERNAL_H__

#include "grandmaster/core.h"

void
read_location(const char *str, struct position *result);

/* Reads PGN data and returns a hierarchy of moves starting at the given start
 * move. If the provided start move is NULL, a new root node is created. */
struct move *
parse_pgn(const char *pgn, int n, struct move *start);

/* Parse FEN data into a move. Private API because it makes a "truncated" move
 * with no hierarchy associated with it. */
struct move *
parse_fen(const char *fen, int n);

/* Print a move to stdout. */
void
print_move(const struct move *);

/* Convert a board to JSON. */
json_t *
board_to_json(const struct board *);

/* Blindly apply movement represented by start and end points, disregarding the
 * validity of the move itself. */
void
apply_movement(struct move *m);

/* Asserts that the given color is either white or black. */
void
assert_valid_color(color_t color);

/* Builds an access map from a move. */
void
build_access_map(struct move *move, struct access_map *out);

/* Frees an access map. */
void
free_access_map(struct access_map *map);

/* Loads the opening position into a board object. */
void
load_default_board(struct board *b);

/* Returns a move that contains the root of the full game tree. */
void
get_root(struct move *out);

/* Free a move struct, leaving its parent move untouched. */
void
free_move(struct move *move);

/* Free a move struct and all of its parents. */
void
free_move_tree(struct move *move);

/* Creates PGN for a given move. In most cases, callers should use
 * move->post_board->pgn instead; this is what is used to populate the pgn field
 * on the board struct. */
char *
create_pgn(struct move *move);

#endif
