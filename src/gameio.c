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

#include "gameio.h"

#include <stdio.h>

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
print_move(struct move *move)
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
