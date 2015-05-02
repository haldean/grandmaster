/*
 * grandmaster_internal.h: private grandmaster APIs
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

/* Blindly apply movement represented by start and end points, disregarding the
 * validity of the move itself. */
void
apply_movement(struct move *m);

/* Asserts that the given color is either white or black. */
void
assert_valid_color(color_t color);

/* Builds an access map from a move. */
void
build_access_map(struct move *move, struct access_map *out);

/* Frees an access map. */
void
free_access_map(struct access_map *map);