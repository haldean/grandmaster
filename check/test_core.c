/*
 * test_rules.py: functional tests for move parsing
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

#include <check.h>
#include <grandmaster/core.h>
#include <grandmaster/internal.h>
#include <stdio.h>
#include <string.h>

#define FAILURE (-1)
#define MOVE_AVAILABLE 0
#define CHECK 1
#define CHECKMATE 2
#define STALEMATE 3

int
check_status(char *fen)
{
    struct move *last;

    last = parse_fen(fen, strlen(fen));
    if (last == NULL) {
        fprintf(stderr, "failed to parse FEN string %s\n", fen);
        return FAILURE;
    }

    if (in_stalemate(last, opposite(last->player)))
        return STALEMATE;
    if (in_checkmate(last, opposite(last->player)))
        return CHECKMATE;
    if (in_check(last, opposite(last->player)))
        return CHECK;
    return MOVE_AVAILABLE;
}

START_TEST(test_in_check)
{
    ck_assert_int_eq(
        CHECK, check_status("3k4/8/8/B2P4/8/8/8/3K4 b - - - -"));
}
END_TEST

int main()
{
    int n_failures;
    Suite *s;
    SRunner *sr;
    TCase *tc;

    s = suite_create("grandmaster");
    tc = tcase_create("in_check");
    tcase_add_test(tc, test_in_check);
    suite_add_tcase(s, tc);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    n_failures = srunner_ntests_failed(sr);
    srunner_free(sr);

    return n_failures;
}
