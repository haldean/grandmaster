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

int
parse_castle(
    const char *notation,
    const struct move *last_move,
    struct move *out)
{
    bool is_kingside;
    bool is_queenside;
    int rook_start_file;
    int rook_end_file;

    is_kingside =
        !strncmp(notation, "0-0", 5) || !strncmp(notation, "O-O", 5);
    is_queenside =
        !strncmp(notation, "0-0-0", 5) || !strncmp(notation, "O-O-O", 5);

    if (!is_kingside && !is_queenside) {
        return 0;
    }

    if (out->player == BLACK) {
        out->start.rank = 7;
        out->end.rank = 7;
    } else {
        out->start.rank = 0;
        out->end.rank = 0;
    }

    out->start.file = 4;
    out->end.file = is_kingside ? 6 : 2;
    rook_start_file = is_kingside ? 7 : 0;
    rook_end_file = is_kingside ? 5 : 3;

    out->post_board = calloc(1, sizeof(struct board));
    memcpy(out->post_board, last_move->post_board, sizeof(struct board));

    /* move the king */
    out->post_board->board[out->end.rank][out->end.file] =
        out->post_board->board[out->start.rank][out->start.file];
    out->post_board->board[out->start.rank][out->start.file] =
        (struct piece) { .color = 0, .piece_type = 0 };

    /* move the rook */
    out->post_board->board[out->end.rank][rook_end_file] =
        out->post_board->board[out->start.rank][rook_start_file];
    out->post_board->board[out->start.rank][rook_start_file] =
        (struct piece) { .color = 0, .piece_type = 0 };

    return 1;
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
    const char *input,
    const struct move *last_move,
    struct move **out)
{
    char *notation;
    bool is_capture;
    size_t capture_index; /* only set if is_capture */
    size_t input_len;
    size_t disambig_len;
    struct move *result;
    struct piece piece;
    int i;

    /* we modify the notation later to ease parsing, so we copy it here to avoid
     * modifying our input. */
    input_len = strlen(input);
    notation = calloc(input_len, sizeof(char));
    memcpy(notation, input, input_len);

    /* create result and fill in known fields */
    result = calloc(1, sizeof(struct move));
    result->player = opposite(last_move->player);
    result->algebraic = calloc(input_len, sizeof(char));
    memcpy(result->algebraic, notation, input_len);
    result->parent = last_move;

    if (input_len < 2) {
        goto error;
    }
    if (parse_castle(notation, last_move, result)) {
        goto done;
    }

    piece.color = result->player;
    piece.piece_type = PAWN;
    for (i = 0; i < sizeof(ALL_PIECES) / sizeof(piece_type_t); i++) {
        if (ALL_PIECES[i] == notation[0]) {
            piece.piece_type = notation[0];
            /* strip the piece off the front of the notation */
            notation++;
            break;
        }
    }

    is_capture = false;
    capture_index = 0;
    for (i = 0; notation[i] != '\0'; i++) {
        if (notation[i] == 'x') {
            is_capture = true;
            capture_index = i;
            break;
        }
    }

    /* strip the check/checkmate annotation, we don't need that to determine the
     * nature of the move. */
    i = strlen(notation) - 1;
    if (notation[i] == '#' || notation[i] == '+') {
        notation[i] = '\0';
    }
    result->end.rank = 0xFF;
    result->end.file = 0xFF;
    read_location(&notation[strlen(notation) - 2], &result->end);
    if (result->end.rank == 0xFF || result->end.file == 0xFF) {
        goto error;
    }

    if (piece.piece_type == PAWN) {
        if (!parse_pawn(notation, last_move, piece, is_capture, result)) {
            goto error;
        }
        goto done;
    }

    if (is_capture) {
        disambig_len = capture_index;
    } else {
        disambig_len = strlen(notation) - 2;
    }

    /* put in sentinel values so we can tell what was initialized (if anything)
     * during disambiguation loading. */
    result->start.rank = 0xFF;
    result->start.file = 0xFF;
    if (disambig_len == 2) {
        /* easiest case: we have two disambig characters that give us the full
         * location of the start piece. */
        read_location(notation, &result->start);
        goto done;
    } else if (disambig_len == 1) {
        if ('a' <= notation[0] && notation[0] <= 'h') {
            result->start.file = notation[0] - 'a';
        } else if ('1' <= notation[0] && notation[0] <= '8') {
            result->start.rank = notation[0] - '1';
        } else {
            goto error;
        }
    }
    goto error;

error:
    free(result);
    *out = NULL;
    return;

done:
    if (result->post_board == NULL) {
        result->post_board = calloc(1, sizeof(struct board));
        memcpy(result->post_board, last_move->post_board, sizeof(struct board));
        result->post_board->board[result->end.rank][result->end.file] =
            result->post_board->board[result->start.rank][result->start.file];
        result->post_board->board[result->start.rank][result->start.file] =
            (struct piece) { .color = 0, .piece_type = 0 };
    }
    *out = result;
}
