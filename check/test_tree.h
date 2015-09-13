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

#include "grandmaster/core.h"
#include "grandmaster/tree.h"

#include <check.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

START_TEST(test_make_move_wrong_player)
{
    struct game_tree *gt;
    game_id_t g1;

    gt = calloc(1, sizeof(struct game_tree));
    init_gametree(gt);

    g1 = new_game(gt, 1, 2);

    ck_assert(!make_move(gt, g1, 2, "e4"));
    ck_assert(make_move(gt, g1, 1, "e4"));

    free_game_tree(gt);
}
END_TEST

START_TEST(test_tree_notation_dedup)
{
    struct game_tree *gt;
    game_id_t g1, g2;

    gt = calloc(1, sizeof(struct game_tree));
    init_gametree(gt);

    g1 = new_game(gt, 12, 56);
    g2 = new_game(gt, 34, 56);

    ck_assert(make_move(gt, g1, 12, "e4"));
    ck_assert(make_move(gt, g2, 34, "e4?"));

    ck_assert_int_eq(2, gt->n_states);
    ck_assert_ptr_eq(gt->games[g1]->current, gt->games[g2]->current);

    free_game_tree(gt);
}
END_TEST

START_TEST(test_tree)
{
    struct game_tree *gt;
    game_id_t g1, g2, g3;

    gt = calloc(1, sizeof(struct game_tree));
    init_gametree(gt);

    g1 = new_game(gt, 12, 56);
    g2 = new_game(gt, 34, 56);
    g3 = new_game(gt, 12, 34);

    ck_assert(make_move(gt, g1, 12, "e4"));
    ck_assert(make_move(gt, g1, 56, "d5"));
    ck_assert(make_move(gt, g2, 34, "e4"));
    ck_assert(make_move(gt, g3, 12, "Nc3"));

    ck_assert_int_eq(4, gt->n_states);
    ck_assert_ptr_eq(gt->games[g1]->current->parent, gt->games[g2]->current);

    free_game_tree(gt);
}
END_TEST

Suite *
make_tree_suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("tree");
    tc = tcase_create("tree");
    tcase_add_test(tc, test_tree);
    tcase_add_test(tc, test_tree_notation_dedup);
    tcase_add_test(tc, test_make_move_wrong_player);
    suite_add_tcase(s, tc);

    return s;
}
