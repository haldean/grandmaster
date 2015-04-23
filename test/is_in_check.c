/*
 * is_in_check.c: returns zero if in check, 1 if not in check, 2 if in checkmate
 * or 3 on failure.
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

#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    struct move *last;

    if (argc < 2) {
        fprintf(stderr, "usage: %s [FEN] [to-play]\n", argv[0]);
        return 3;
    }

    last = parse_fen(argv[1], strlen(argv[1]));
    if (last == NULL) {
        fprintf(stderr, "failed to parse FEN string %s\n", argv[1]);
        return 3;
    }

    if (in_checkmate(last))
        return 2;
    if (in_check(last))
        return 0;
    return 1;
}
