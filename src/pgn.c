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

#include "gameio.h"
#include <string.h>

#define MAX_NOTATION_LEN 63

#ifdef DEBUG
#  define pgn_fail(...) do {\
        printf("pgn_fail: "); \
        printf(__VA_ARGS__); \
        printf("\n"); \
        goto error; } while (0);
#else
#  define pgn_fail(...) do { goto error; } while (0);
#endif

int
read_ply(const char *pgn, const int n, int *const i, char *const notation)
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
    if (*i == n || notation_i == MAX_NOTATION_LEN)
        return 1;
    notation[notation_i] = '\0';
    return 0;
}

int
read_termination(const char *pgn, const int n, int i)
{
    // first consume all whitespace
    while ((pgn[i] == '\n' || pgn[i] == ' ') && i < n)
        i++;
    if (i == n)
        return 1;

    if (n - i - 1 < 3)
        return 1;
    if (strncmp(&pgn[i], "1-0", 3) == 0)
        return 0;
    if (strncmp(&pgn[i], "0-1", 3) == 0)
        return 0;
    if (n - i - 1 < 7)
        return 1;
    if (strncmp(&pgn[i], "1/2-1/2", 3) == 0)
        return 0;
    return 1;
}

struct move *
parse_pgn(const char *pgn, int n, struct move *start)
{
    struct move *last;
    struct move *next;
    char notation[MAX_NOTATION_LEN+1];
    int i;
    int err;

    if (start == NULL) {
        last = calloc(1, sizeof(struct move));
        get_root(last);
    } else {
        last = start;
    }

    for (i = 0; i < n; i++) {
        // strip out whitespace and metadata before reading the move
        while ((pgn[i] == ' ' || pgn[i] == '\n' || pgn[i] == '[') && i < n) {
            // ignore PGN metadata by nongreedily consuming until the closing square
            // bracket.
            if (pgn[i] == '[') {
                while (pgn[i] != ']' && i < n)
                    i++;
                if (i == n)
                    pgn_fail("couldn't find matching square bracket");
            }
            i++;
        }
        if (i == n)
            return last;
        // find the next period, which marks the start of the move. at the end
        // of this chunk, i points to the character after the period.
        while (pgn[i] != '.' && i < n)
            i++;
        if (i == n)
            return last;
        i++;

        err = read_ply(pgn, n, &i, notation);
        if (err)
            pgn_fail("failed to read white ply at i = %d", i)

        parse_algebraic(notation, last, &next);
        if (next == NULL)
            pgn_fail("failed to parse white ply %s", notation)
        last = next;

        err = read_termination(pgn, n, i);
        if (!err)
            return last;

        err = read_ply(pgn, n, &i, notation);
        if (err)
            pgn_fail("failed to read black ply at i = %d", i)

        parse_algebraic(notation, last, &next);
        if (next == NULL)
            pgn_fail("failed to parse black ply %s", notation)
        last = next;

        err = read_termination(pgn, n, i);
        if (!err)
            return last;
    }

    return last;

error:
    if (last != NULL)
        free_move_tree(last);
    return NULL;
}
