/*
 * gameio.h: tools for saving and loading game data
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

#ifndef __GAMEIO_H__
#define __GAMEIO_H__

#include "grandmaster.h"
#include "jansson.h"

void
read_location(const char *str, struct position *result);

struct move *
parse_pgn(const char *pgn, int n);

struct move *
parse_fen(const char *fen, int n);

void
print_move(const struct move *);

json_t *
move_to_json(const struct move *);

json_t *
board_to_json(const struct board *);

#endif
