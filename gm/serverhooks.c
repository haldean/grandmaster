/*
 * serverhooks.c: handling server requests
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


#include <grandmaster/core.h>
#include <grandmaster/tree.h>
#include <grandmaster/gmutil.h>

#include <jansson.h>

#define get(t, req, field) \
    t = json_object_get(req, field); \
    if (!t) return json_pack("{ss}", "error", "missing field " field);

json_t *
game_state(struct game_tree *gt, game_id_t game)
{
    (void)(gt);
    (void)(game);
    return json_null();
}

json_t *
handle_new_game(struct game_tree *gt, json_t *req)
{
    game_id_t game;
    player_id_t white;
    player_id_t black;
    json_t *t;

    get(t, req, "player_white");
    white = json_integer_value(t);

    get(t, req, "player_black");
    black = json_integer_value(t);

    game = new_game(gt, white, black);
    return json_pack("{sIsosn}",
            "game_id", game,
            "state", game_state(gt, game),
            "error" /* undefined */);
}

json_t *
handle_move(struct game_tree *gt, json_t *req)
{
    game_id_t game_id;
    player_id_t player;
    const char *notation;
    json_t *t;
    bool success;

    get(t, req, "player");
    player = json_integer_value(t);

    get(t, req, "game_id");
    game_id = json_integer_value(t);

    get(t, req, "move");
    notation = json_string_value(t);

    success = make_move(gt, game_id, notation);
    if (success) {
        return json_pack("{sosn}", "state", game_state(gt, game_id), "error");
    }
    return json_pack("{snss}", "state", "error", "could not perform move");
}