/*
 * algebraic.c: parse algebraic notation
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
#include "gameio.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

void
parse_castle(
    const char *notation,
    const struct move *last_move,
    const bool is_kingside,
    struct move *out)
{
    if (out->player == BLACK) {
        out->start.rank = 7;
        out->end.rank = 7;
    } else {
        out->start.rank = 0;
        out->end.rank = 0;
    }

    out->start.file = 4;
    out->end.file = is_kingside ? 6 : 2;
}

int
parse_pawn(
    const char *notation,
    const struct move *last_move,
    const struct piece piece,
    const bool is_capture,
    struct move *out)
{
    /* TODO */
    return 1;
}

void
parse_algebraic(
    const char *notation,
    const struct move *last_move,
    struct move **out)
{
    bool is_kingside_castle;
    bool is_queenside_castle;
    bool is_capture;
    size_t input_len;
    struct move *result;
    struct piece piece;
    int i;

    input_len = strlen(notation);

    /* create result and fill in known fields */
    result = calloc(1, sizeof(struct move));
    result->player = opposite(last_move->player);
    result->algebraic = calloc(input_len, sizeof(char));
    memcpy(result->algebraic, notation, input_len);
    result->parent = last_move;

    if (input_len < 2) {
        goto error;
    }

    is_kingside_castle =
        strncmp(notation, "0-0", 3) || strncmp(notation, "O-O", 3);
    is_queenside_castle =
        strncmp(notation, "0-0-0", 5) || strncmp(notation, "O-O-O", 5);
    if (is_kingside_castle || is_queenside_castle) {
        parse_castle(notation, last_move, is_kingside_castle, result);
        goto done;
    }

    piece.color = result->player;
    piece.piece_type = PAWN;
    for (i = 0; i < sizeof(ALL_PIECES) / sizeof(piece_type_t); i++) {
        if (ALL_PIECES[i] == notation[0]) {
            piece.piece_type = notation[0];
            notation++;
            break;
        }
    }

    is_capture = false;
    for (i = 0; i < input_len; i++) {
        if (notation[i] == 'x') {
            is_capture = true;
            break;
        }
    }

    /* strip the check/checkmate annotation, we don't need that to determine the
     * nature of the move. */

    read_location(&notation[input_len - 2], &result->end);

    if (piece.piece_type == PAWN) {
        if (!parse_pawn(notation, last_move, piece, is_capture, result)) {
            goto error;
        }
        goto done;
    }

error:
    free(result);
    result = NULL;
    return;

done:
    *out = result;
}
