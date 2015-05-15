/*
 * tree.h: game-tree-related functions and data
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

#ifndef __GRANDMASTER_TREE_H__
#define __GRANDMASTER_TREE_H__

#include "grandmaster/core.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint64_t game_id_t;
typedef uint64_t player_id_t;

#define NO_GAME ((game_id_t) -1)

struct state_node {
    struct move *move;
    size_t n_children;
    struct state_node **children;
    struct state_node *parent;
};

struct game {
    game_id_t id;
    player_id_t player_white;
    player_id_t player_black;
    struct state_node *current;
    termination_t termination;
};

struct game_tree {
    size_t n_states;
    struct state_node **states;
    size_t n_games;
    struct game **games;
};

void
init_gametree(struct game_tree *gt);

game_id_t
new_game(struct game_tree *gt, player_id_t white, player_id_t black);

struct game *
get_game(struct game_tree *gt, game_id_t game);

bool
make_move(
    struct game_tree *gt,
    game_id_t game,
    const char *notation);

game_id_t
new_game_from_pgn(
    struct game_tree *gt,
    player_id_t white,
    player_id_t black,
    const char *pgn);

void
free_game_tree(struct game_tree *gt);

json_t *
game_tree_to_json(struct game_tree *gt);

void
game_tree_from_json(json_t *doc, struct game_tree *gt);

#endif
