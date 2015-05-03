/*
 * move_parser.c: test algebraic notation parsing
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
#include "gametree.h"

#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main()
{
    struct game_tree *gt;
    json_t *results;

    gt = calloc(1, sizeof(struct game_tree));
    init_gametree(gt);

    results = game_tree_to_json(gt);
    json_dumpf(results, stdout, JSON_PRESERVE_ORDER | JSON_INDENT(2));
    printf("\n");

    return 0;
}
