/*
 * fen.c: parser for the FEN data interchange format
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NOTATION_LEN 63

#ifdef DEBUG
#  define fen_fail(...) do {\
        printf("fen_fail: "); \
        printf(__VA_ARGS__); \
        printf("\n"); \
        goto error; } while (0);
#else
#  define fen_fail(...) do { goto error; } while (0);
#endif

int
load_piece(const char piece, struct piece *board)
{
    piece_type_t piece_code;
    size_t i;

    if ('A' <= piece && piece <= 'Z')
        board->color = WHITE;
    else if ('a' <= piece && piece <= 'z')
        board->color = BLACK;
    else
        return 0;
    if (piece == 'p' || piece == 'P') {
        board->piece_type = PAWN;
        return 1;
    }

    piece_code = (piece_type_t) toupper(piece);
    board->piece_type = 0;
    for (i = 0; i < sizeof(ALL_PIECES) / sizeof(piece_type_t); i++) {
        if (ALL_PIECES[i] == piece_code) {
            board->piece_type = piece_code;
        }
    }
    return board->piece_type != 0;
}

struct move *
parse_fen(const char *fen, int n)
{
    struct move *result;
    struct board *board;
    int i;
    int rank;
    int file;

    result = calloc(1, sizeof(struct move));
    board = calloc(1, sizeof(struct board));
    result->post_board = board;

    /* read board */
    i = 0;
    for (rank = 7; rank >= 0; rank--) {
        for (file = 0; file < 8; file++) {
            if (i >= n)
                fen_fail("ran out of characters");
            if ('1' <= fen[i] && fen[i] <= '9')
                file += fen[i++] - '1';
            else if (!load_piece(fen[i++], &board->board[rank][file]))
                fen_fail("couldn't load piece %c", fen[i-1]);
        }
        if (fen[i++] != '/' && rank != 0)
            fen_fail("didn't find slash at end of rank");
    }
    if (i >= n)
        fen_fail("ran out of characters");

    /* read next-to-play, which shows up in the resulting move as the opposite
     * of what's in the FEN, since here we're recording who moved last. */
    result->player = opposite(fen[i++]);
    if (i >= n)
        fen_fail("ran out of characters");

    /* skip the space separator */
    i++;
    if (i >= n)
        fen_fail("ran out of characters");

    /* read available castles */
    board->available_castles = 0;
    while (fen[i] != ' ') {
        if (fen[i] == 'K')
            board->available_castles |= WHITE_KINGSIDE;
        else if (fen[i] == 'k')
            board->available_castles |= BLACK_KINGSIDE;
        else if (fen[i] == 'Q')
            board->available_castles |= WHITE_QUEENSIDE;
        else if (fen[i] == 'q')
            board->available_castles |= BLACK_QUEENSIDE;
        else if (fen[i] == '-')
            ; /* do nothing */
        else
            fen_fail("unknown castle type %c", fen[i]);
        i++;
        if (i >= n)
            fen_fail("ran out of characters");
    }

    /* TODO: en passant square */

    return result;

error:
    free_move(result);
    return NULL;
}
