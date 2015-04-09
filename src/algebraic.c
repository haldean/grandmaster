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

#ifdef DEBUG
#  define alg_fail(...) do {\
        printf("alg_fail: "); \
        printf(__VA_ARGS__); \
        printf("\n"); \
        goto error; } while (0);
#else
#  define alg_fail(...) do { goto error; } while (0);
#endif

inline bool
is_valid_position(const struct position pos)
{
    return 0 <= pos.rank && pos.rank < 8 && 0 <= pos.file && pos.file < 8;
}

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
    out->start.rank = 0;
    out->end.file = notation[strlen(notation) - 2] - 'a';
    out->end.rank = notation[strlen(notation) - 1] - '1';
    if (!is_valid_position(out->start))
        return 0;
    if (!is_valid_position(out->end))
        return 0;

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

void
parse_algebraic(
    const char *input,
    const struct move *last_move,
    struct move **out)
{
    char *notation;
    char *notation_head;
    bool is_capture;
    size_t capture_index; /* only set if is_capture */
    size_t input_len;
    size_t disambig_len;
    struct move *result;
    struct piece piece;
    int i;

    /* we modify the notation later to ease parsing, so we copy it here to avoid
     * modifying our input. */
    input_len = strlen(input) + 1;
    notation = calloc(input_len, sizeof(char));
    notation_head = notation;
    strncpy(notation, input, input_len);

    /* create result and fill in known fields */
    result = calloc(1, sizeof(struct move));
    result->player = opposite(last_move->player);
    result->algebraic = calloc(input_len, sizeof(char));
    strncpy(result->algebraic, notation, input_len);
    result->parent = last_move;

    if (input_len < 2) {
        alg_fail("input too short");
    }
    if (parse_castle(notation, last_move, result)) {
        piece.piece_type = KING;
        piece.color = result->player;
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
        alg_fail("end location didn't parse");
    }

    if (piece.piece_type == PAWN) {
        if (parse_pawn(notation, last_move, is_capture, result)) {
            goto done;
        }
        alg_fail("failed to parse pawn movement");
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
            alg_fail("invalid disambig char %c", notation[0]);
        }
    }

    find_piece_with_access(piece, result);
    if (result->start.rank == -1 || result->end.rank == -1)
        alg_fail("no pieces with access to end location");

done:
    if (!is_movement_valid(result)) {
        alg_fail("movement wasn't valid");
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

    free(notation_head);
    *out = result;
    return;

error:
    free(notation_head);
    free_move(result);
    *out = NULL;
    return;
}
