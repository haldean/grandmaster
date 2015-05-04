/*
 * gameio.c: tools for saving and loading game data
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
#include "grandmaster/internal.h"
#include "grandmaster/tree.h"

#include <assert.h>
#include <jansson.h>
#include <stdio.h>

#define json_set json_object_set_new_nocheck

void
read_location(const char *str, struct position *result)
{
    if ('a' <= str[0] && str[0] <= 'h' && '1' <= str[1] && str[1] <= '8') {
        result->file = str[0] - 'a';
        result->rank = str[1] - '1';
    }
}

char
rank_str(struct position position)
{
    return position.rank + '1';
}

char
file_str(struct position position)
{
    return position.file + 'a';
}

char *
player_str(color_t player)
{
    return player == WHITE ? "white" : "black";
}

void
print_move(const struct move *move)
{
    int rank;
    int file;
    struct piece *p;

    if (move->algebraic != NULL) {
        printf("%s\n", move->algebraic);
        printf("%c%c -> %c%c\n",
               file_str(move->start), rank_str(move->start),
               file_str(move->end), rank_str(move->end));
        printf("%s moved\n", player_str(move->player));
    }

    if (move->post_board == NULL) {
        printf("no board.\n");
        return;
    }
    for (rank = 7; rank >= 0; rank--) {
        for (file = 0; file < 8; file++) {
            p = &move->post_board->board[rank][file];
            if (p->piece_type != 0)
                printf("%c%c  ", (char) p->color, (char) p->piece_type);
            else
                printf("__  ");
        }
        printf("\n");
    }
}

json_t *
board_to_json(const struct board *board)
{
    int rank;
    int file;
    const struct piece *p;
    json_t *board_root;
    json_t *board_array;
    json_t *rank_array;
    json_t *available_castles;
    json_t *temp;
    char piece_name[3];

    board_root = json_object();
    board_array = json_array();

    for (rank = 0; rank < 8; rank++) {
        rank_array = json_array();
        for (file = 0; file < 8; file++) {
            p = &board->board[rank][file];
            if (p->piece_type == 0) {
                temp = json_null();
            } else {
                snprintf(
                    piece_name, 3, "%c%c",
                    (char) p->color, (char) p->piece_type);
                temp = json_string(piece_name);
            }
            json_array_append_new(rank_array, temp);
        }
        json_array_append_new(board_array, rank_array);
    }

    json_set(board_root, "board", board_array);

    available_castles = json_integer(board->available_castles);
    json_set(
        board_root, "available_castles", available_castles);

    return board_root;
}

json_t *
move_to_json(const struct move *move)
{
    json_t *root;
    json_t *temp;

    root = json_object();

    if (move->algebraic != NULL) {
        temp = json_string(move->algebraic);
    } else {
        temp = json_null();
    }
    json_set(root, "algebraic", temp);

    if (move->post_board != NULL) {
        temp = board_to_json(move->post_board);
    } else {
        temp = json_null();
    }
    json_set(root, "board", temp);

    temp = json_integer(move->start.rank);
    json_set(root, "start_rank", temp);
    temp = json_integer(move->start.file);
    json_set(root, "start_file", temp);

    temp = json_integer(move->end.rank);
    json_set(root, "end_rank", temp);
    temp = json_integer(move->end.file);
    json_set(root, "end_file", temp);

    return root;
}

json_t *
game_tree_to_json(struct game_tree *gt)
{
    size_t i;
    size_t j;
    struct move *move;
    struct game *game;
    json_t *out;
    json_t *states;
    json_t *games;
    json_t *t;
    bool found_parent;

    out = json_object();
    states = json_array();
    json_set(out, "states", states);
    games = json_array();
    json_set(out, "games", games);

    for (i = 0; i < gt->n_states; i++) {
        move = gt->states[i]->move;
        t = move_to_json(move);
        json_set(t, "id", json_integer(i));
        if (move->parent == NULL) {
            json_set(t, "parent", json_null());
        } else {
            found_parent = false;
            for (j = 0; j < gt->n_states; j++) {
                if (gt->states[j]->move == move->parent) {
                    json_set(t, "parent", json_integer(j));
                    found_parent = true;
                    break;
                }
            }
            assert(found_parent);
        }
        json_array_append_new(states, t);
    }

    for (i = 0; i < gt->n_games; i++) {
        game = gt->games[i];
        t = json_object();
        json_set(t, "id", json_integer(game->id));
        json_set(t, "white", json_integer(game->player_white));
        json_set(t, "black", json_integer(game->player_black));

        found_parent = false;
        for (j = 0; j < gt->n_states; j++) {
            if (gt->states[j]->move == game->current->move) {
                json_set(t, "current", json_integer(j));
                found_parent = true;
                break;
            }
        }
        assert(found_parent);
        json_array_append_new(games, t);
    }

    return out;
}
