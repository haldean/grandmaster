/*
 * pgn.c: parser for the PGN data interchange format
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NOTATION_LEN 63

#ifdef DEBUG
#  define pgn_fail(...) do {\
        printf("pgn_fail: "); \
        printf(__VA_ARGS__); \
        printf("\n"); \
        goto error; } while (0)
#else
#  define pgn_fail(...) do { goto error; } while (0)
#endif

int
read_ply(const char *pgn, const size_t n, size_t *const i, char *const notation)
{
    int notation_i;

    // now consume all whitespace
    while ((pgn[*i] == '\n' || pgn[*i] == ' ') && *i < n)
        (*i)++;
    if (*i == n)
        return 1;

    // we're now at the next ply of the move. copy chars until we hit
    // whitespace.
    notation_i = 0;
    while (pgn[*i] != '\n' && pgn[*i] != ' '
            && *i < n && notation_i < MAX_NOTATION_LEN)
        notation[notation_i++] = pgn[(*i)++];
    if (notation_i == MAX_NOTATION_LEN)
        return 1;
    notation[notation_i] = '\0';
    return 0;
}

int
read_termination(
        struct game_tree *gt,
        game_id_t game_id,
        const char *pgn,
        const int n,
        int i)
{
    termination_t termination;

    /* first consume all whitespace */
    while ((pgn[i] == '\n' || pgn[i] == ' ') && i < n)
        i++;
    if (i == n)
        return 1;

    if (strncmp(&pgn[i], "1-0", 3) == 0)
        termination = VICTORY_WHITE;
    else if (strncmp(&pgn[i], "0-1", 3) == 0)
        termination = VICTORY_BLACK;
    else if (strncmp(&pgn[i], "1/2-1/2", 7) == 0)
        termination = STALEMATE;
    else
        return 1;

    if (end_game(gt, game_id, termination))
        return 0;
    return 1;
}

game_id_t
new_game_from_pgn(
    struct game_tree *gt,
    player_id_t white,
    player_id_t black,
    const char *pgn)
{
    char notation[MAX_NOTATION_LEN+1];
    size_t i;
    size_t n;
    int err;
    bool success;
    game_id_t game_id;

    game_id = new_game(gt, white, black);
    n = strlen(pgn);
    for (i = 0; i < n; i++) {
        /* strip out whitespace and metadata before reading the move */
        while ((pgn[i] == ' ' || pgn[i] == '\n' || pgn[i] == '[') && i < n) {
            /* ignore PGN metadata by nongreedily consuming until the closing
             * square bracket. */
            if (pgn[i] == '[') {
                while (pgn[i] != ']' && i < n)
                    i++;
                if (i == n)
                    pgn_fail("couldn't find matching square bracket");
            }
            i++;
        }
        if (i == n)
            return game_id;
        /* find the next period, which marks the start of the move. at the end
         * of this chunk, i points to the character after the period. */
        while (pgn[i] != '.' && i < n)
            i++;
        if (i == n)
            return game_id;
        i++;

        err = read_ply(pgn, n, &i, notation);
        if (err)
            pgn_fail("failed to read white ply at i = %lu", i);
        success = make_move(gt, game_id, white, notation);
        if (!success)
            pgn_fail("failed to parse white ply %s", notation);
        if (i == n)
            return game_id;

        /* TODO: use this! */
        err = read_termination(gt, game_id, pgn, n, i);
        if (!err)
            return game_id;

        err = read_ply(pgn, n, &i, notation);
        if (err)
            pgn_fail("failed to read black ply at i = %lu", i);
        success = make_move(gt, game_id, black, notation);
        if (!success)
            pgn_fail("failed to parse black ply %s", notation);

        /* TODO: use this! */
        err = read_termination(gt, game_id, pgn, n, i);
        if (!err)
            return game_id;
    }

    return game_id;

error:
    return NO_GAME;
}

char *
create_pgn(struct move *move)
{
    char *base;
    char *res;
    size_t base_len;
    int ret;

    /* if we're the root node, there's no need to include us in the PGN. */
    if (move->algebraic == NULL)
        return "";

    if (move->parent != NULL) {
        base = create_pgn(move->parent);
        base_len = strlen(base);
    } else {
        base = "";
        base_len = 0;
    }

    /* propagate errors back up the stack. */
    if (base == NULL)
        return NULL;

    if (move->player == WHITE) {
        if (base_len > 0)
            ret = asprintf(&res, "%s %d.%s",
                base,
                move->post_board->ply_index / 2 + 1,
                move->algebraic);
        else
            ret = asprintf(&res, "%d.%s",
                move->post_board->ply_index / 2 + 1,
                move->algebraic);
    } else {
        ret = asprintf(&res, "%s %s", base, move->algebraic);
    }

    if (ret == -1) {
        perror("E: couldn't asprintf PGN data");
        return NULL;
    }
    return res;
}
