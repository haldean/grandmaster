/*
 * gametree.h: game-tree-related functions and data
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
#include <stdbool.h>
#include <stdint.h>

typedef uint64_t game_id_t;
typedef uint64_t player_id_t;

struct game {
    game_id_t id;
    player_id_t player_white;
    player_id_t player_black;
    struct move *current;
};

struct game_tree {
    size_t n_states;
    struct move *states;
    size_t n_games;
    struct game *games;
}

struct game *
new_game(player_id_t white, player_id_t black);

struct game *
get_game(game_id_t game);

bool
make_move(
    struct game_tree *tree,
    struct game *game,
    const char *notation);
