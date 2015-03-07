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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
parse_castle(
    const char *notation,
    const struct move *last_move,
    struct move *out)
{
    bool is_kingside;
    castles_t castle_type;
    int rook_start_file;
    int rook_end_file;

    castle_type = 0;
    if (!strncmp(notation, "0-0", 5) || !strncmp(notation, "O-O", 5)) {
        castle_type = out->player == WHITE ? WHITE_KINGSIDE : BLACK_KINGSIDE;
    }
    else if (!strncmp(notation, "0-0-0", 5) || !strncmp(notation, "O-O-O", 5)) {
        castle_type = out->player == WHITE ? WHITE_QUEENSIDE : BLACK_QUEENSIDE;
    }

    if (!castle_type)
        return 0;
    if (!(last_move->post_board->available_castles & castle_type))
        return 0;

    if (out->player == BLACK) {
        out->start.rank = 7;
        out->end.rank = 7;
    } else {
        out->start.rank = 0;
        out->end.rank = 0;
    }

    is_kingside = castle_type & (WHITE_KINGSIDE | BLACK_KINGSIDE);
    out->start.file = 4;
    out->end.file = is_kingside ? 6 : 2;
    rook_start_file = is_kingside ? 7 : 0;
    rook_end_file = is_kingside ? 5 : 3;

    out->post_board = calloc(1, sizeof(struct board));
    /* update available castles */
    out->post_board->available_castles =
        last_move->post_board->available_castles;
    if (out->player == BLACK) {
        out->post_board->available_castles &=
            ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
    } else {
        out->post_board->available_castles &=
            ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
    }

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
    const bool is_capture,
    struct move *out)
{
    color_t player;
    struct board *b;

    player = opposite(last_move->player);
    b = last_move->post_board;

    out->start.file = notation[0] - 'a';
    out->end.file = notation[strlen(notation) - 2] - 'a';
    out->end.rank = notation[strlen(notation) - 1] - '1';

    if (is_capture) {
        if (player == WHITE) {
            out->start.rank = out->end.rank - 1;
        } else {
            out->start.rank = out->end.rank + 1;
        }
        return 1;
    }

    if (player == WHITE) {
        if (out->end.rank == 3) {
            if (b->board[2][out->start.file].color == WHITE) {
                out->start.rank = 2;
            } else {
                out->start.rank = 1;
            }
        } else {
            out->start.rank = out->end.rank - 1;
        }
    } else {
        if (out->end.rank == 4) {
            if (b->board[5][out->start.file].color == BLACK) {
                out->start.rank = 5;
            } else {
                out->start.rank = 6;
            }
        } else {
            out->start.rank = out->end.rank + 1;
        }
    }
    return 1;
}

/* Finds a piece of the given color and type that has access to move->end,
 * respecting any preexisting values in move->start. Populates the value at
 * move->start with results if any are found. If none are found, move is
 * entirely unchanged. */
void
find_piece_with_access(struct piece piece, struct move *move)
{
    int8_t rank;
    int8_t file;
    struct move test_move;
    struct piece *board_piece;

    test_move.parent = move->parent;
    test_move.end = move->end;

    for (rank = 0; rank < 8; rank++) {
        /* we could do this more efficiently by skipping the loop altogether in
         * this case, but I like the succinctness of doing this all in one loop
         * with no special cases. */
        if (move->start.rank != -1 && move->start.rank != rank)
            continue;
        for (file = 0; file < 8; file++) {
            if (move->start.file != -1 && move->start.file != file)
                continue;
            board_piece = &move->parent->post_board->board[rank][file];
            if (board_piece->piece_type != piece.piece_type)
                continue;
            if (board_piece->color != piece.color)
                continue;
            test_move.start.rank = rank;
            test_move.start.file = file;
            if (is_movement_valid(&test_move)) {
                move->start = test_move.start;
                return;
            }
        }
    }
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
    for (i = 0; i < (int) (sizeof(ALL_PIECES) / sizeof(piece_type_t)); i++) {
        if ((int) ALL_PIECES[i] == notation[0]) {
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
    result->end.rank = -1;
    result->end.file = -1;
    read_location(&notation[strlen(notation) - 2], &result->end);
    if (result->end.rank == -1 || result->end.file == -1) {
        goto error;
    }

    if (piece.piece_type == PAWN) {
        if (parse_pawn(notation, last_move, is_capture, result)) {
            goto done;
        }
        goto error;
    }

    if (is_capture) {
        disambig_len = capture_index;
    } else {
        disambig_len = strlen(notation) - 2;
    }

    /* put in sentinel values so we can tell what was initialized (if anything)
     * during disambiguation loading. */
    result->start.rank = -1;
    result->start.file = -1;
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

    find_piece_with_access(piece, result);
    if (result->start.rank == -1 || result->end.rank == -1)
        goto error;

done:
    if (!is_movement_valid(result)) {
        goto error;
    }

    if (result->post_board == NULL) {
        result->post_board = calloc(1, sizeof(struct board));
        memcpy(result->post_board, last_move->post_board, sizeof(struct board));
        result->post_board->board[result->end.rank][result->end.file] =
            result->post_board->board[result->start.rank][result->start.file];
        result->post_board->board[result->start.rank][result->start.file] =
            (struct piece) { .color = 0, .piece_type = 0 };
    }

    if (piece.piece_type == ROOK || piece.piece_type == KING) {
        if (result->player == WHITE) {
            result->post_board->available_castles &=
                ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
        } else {
            result->post_board->available_castles &=
                ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
        }
    }

    *out = result;
    return;

error:
    free(result);
    *out = NULL;
    return;
}
