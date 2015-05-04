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
#include "grandmaster_internal.h"
#include "gametree.h"

#include <stdlib.h>
#include <string.h>

void
init_gametree(struct game_tree *gt)
{
    gt->n_states = 1;
    gt->states = calloc(1, sizeof(struct state_node));
    gt->n_games = 0;
    gt->games = NULL;

    gt->states[0].n_children = 0;
    gt->states[0].children = NULL;
    gt->states[0].move = calloc(1, sizeof(struct move));
    get_root(gt->states[0].move);
}

game_id_t
new_game(struct game_tree *gt, player_id_t white, player_id_t black)
{
    struct game **new_games;
    int i;

    new_games = realloc(gt->games, (gt->n_games + 1) * sizeof(struct game *));
    if (new_games == NULL) {
        return NO_GAME;
    }
    i = gt->n_games;
    gt->games = new_games;
    gt->n_games++;

    gt->games[i] = calloc(1, sizeof(struct game));
    gt->games[i]->id = i;
    gt->games[i]->player_white = white;
    gt->games[i]->player_black = black;
    gt->games[i]->current = &gt->states[0];

    return gt->games[i]->id;
}

bool
moves_equivalent(struct move *m1, struct move *m2)
{
    if (m1->parent != m2->parent)
        return false;
    if (m1->start.rank != m2->start.rank)
        return false;
    if (m1->start.file != m2->start.file)
        return false;
    if (m1->end.rank != m2->end.rank)
        return false;
    if (m1->end.file != m2->end.file)
        return false;
    return true;
}

bool
make_move(
    struct game_tree *gt,
    game_id_t game_id,
    const char *notation)
{
    struct game *game;
    struct move *move;
    struct move *t;
    struct state_node **new_children;
    struct state_node *new_states;
    bool existing;
    size_t i;
    size_t j;

    game = get_game(gt, game_id);

    move = NULL;
    parse_algebraic(notation, game->current->move, &move);
    if (move == NULL)
        return false;

    existing = false;
    for (i = 0; i < game->current->n_children; i++) {
        t = game->current->children[i]->move;
        if (moves_equivalent(move, t)) {
            free_move(move);
            game->current = game->current->children[i];
            return true;
        }
    }

    new_states = realloc(
        gt->states,
        (gt->n_states + 1) * sizeof(struct state_node));
    if (new_states == NULL)
        return false;
    i = gt->n_states;
    gt->n_states++;
    gt->states = new_states;

    gt->states[i].move = move;
    gt->states[i].n_children = 0;
    gt->states[i].children = NULL;

    new_children = realloc(
        game->current->children,
        (game->current->n_children + 1) * sizeof(struct state_node *));
    if (new_children == NULL)
        return false;
    j = game->current->n_children;
    game->current->n_children++;
    game->current->children[j] = &gt->states[i];

    game->current = &gt->states[i];
    return true;
}

struct game *
get_game(struct game_tree *gt, game_id_t game)
{
    size_t i;
    for (i = 0; i < gt->n_games; i++) {
        if (gt->games[i]->id == game)
            return gt->games[i];
    }
    return NULL;
}

void
get_root(struct move *out)
{
    memset(out, 0, sizeof(struct move));
    out->player = BLACK;
    out->post_board = calloc(1, sizeof(struct board));
    load_default_board(out->post_board);
    build_access_map(out, out->post_board->access_map);
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
