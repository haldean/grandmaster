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

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

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

struct move *
apply_moves_to_fen(char *fen, size_t n_moves, char *moves[])
{
    struct move *last;
    struct move *next;
    size_t i;

    last = parse_fen(fen, strlen(fen));
    for (i = 0; i < n_moves; i++) {
        parse_algebraic(moves[i], last, &next);
        if (next == NULL)
            return NULL;
        last = next;
    }

    return last;
}

START_TEST(test_pawn)
{
    struct move *res;
    char *move1[1];
    char *move2[2];
    char *move3[3];

    move1[0] = "a4";
    res = apply_moves_to_fen(START_FEN, 1, move1);
    ck_assert_ptr_ne(res, NULL);
    move1[0] = "a3";
    res = apply_moves_to_fen(START_FEN, 1, move1);
    ck_assert_ptr_ne(res, NULL);
    move1[0] = "a5";
    res = apply_moves_to_fen(START_FEN, 1, move1);
    ck_assert_ptr_eq(res, NULL);

    move2[0] = "a4"; move2[1] = "b5";
    res = apply_moves_to_fen(START_FEN, 2, move2);
    ck_assert_ptr_ne(res, NULL);

    move3[0] = "a4"; move3[1] = "b5"; move3[2] = "axb5";
    res = apply_moves_to_fen(START_FEN, 3, move3);
    ck_assert_ptr_ne(res, NULL);
    move3[0] = "a4"; move3[1] = "c5"; move3[2] = "axb5";
    res = apply_moves_to_fen(START_FEN, 3, move3);
    ck_assert_ptr_eq(res, NULL);
}
END_TEST

START_TEST(test_bishop)
{
    struct move *res;
    char *move4[] = {"a4", "b5", "a5", "Ba6"};
    char *move6[] = {"a4", "b5", "a5", "b6", "a6", "Ba6"};

    res = apply_moves_to_fen(START_FEN, 4, move4);
    ck_assert_ptr_ne(res, NULL);
    res = apply_moves_to_fen(START_FEN, 6, move6);
    ck_assert_ptr_eq(res, NULL);
}
END_TEST

START_TEST(test_castle)
{
    struct move *res;
    char *ks_castle[1] = {"O-O"};
    char *qs_castle[1] = {"O-O-O"};
    char *qs_rook_ks_castle[3] = {"Rb8", "c3", "O-O"};
    char *qs_rook_qs_castle[3] = {"Rb8", "c3", "O-O-O"};
    char *ks_rook_ks_castle[3] = {"Rg8", "c3", "O-O"};
    char *ks_rook_qs_castle[3] = {"Rg8", "c3", "O-O-O"};

    res = apply_moves_to_fen(
        "r3kbnr/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b q - - -",
        1, qs_castle);
    ck_assert_ptr_ne(res, NULL);
    ck_assert_str_eq(
        res->post_board->fen,
        "2kr1bnr/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR w - - - 1");

    /* castle not available */
    res = apply_moves_to_fen(
        "r3kbnr/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b - - - -",
        1, qs_castle);
    ck_assert_ptr_eq(res, NULL);

    res = apply_moves_to_fen(
        "r3k2r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b kq - - -",
        1, ks_castle);
    ck_assert_ptr_ne(res, NULL);
    ck_assert_str_eq(
        res->post_board->fen,
        "r4rk1/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR w - - - 1");

    res = apply_moves_to_fen(
        "r3k2r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b kq - - -",
        1, qs_castle);
    ck_assert_ptr_ne(res, NULL);
    ck_assert_str_eq(
        res->post_board->fen,
        "2kr3r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR w - - - 1");

    res = apply_moves_to_fen(
        "r3k2r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b kq - - -",
        1, ks_castle);
    ck_assert_ptr_ne(res, NULL);
    ck_assert_str_eq(
        res->post_board->fen,
        "r4rk1/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR w - - - 1");

    res = apply_moves_to_fen(
        "r3k2r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b kq - - -",
        3, qs_rook_ks_castle);
    ck_assert_ptr_ne(res, NULL);
    ck_assert_str_eq(
        res->post_board->fen,
        "1r3rk1/p1pp1ppp/8/8/1p6/P1P1p3/1P1PPPPP/RNBQKBNR w - - - 2");

    res = apply_moves_to_fen(
        "r3k2r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b kq - - -",
        3, qs_rook_qs_castle);
    ck_assert_ptr_eq(res, NULL);

    res = apply_moves_to_fen(
        "r3k2r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b kq - - -",
        3, ks_rook_qs_castle);
    ck_assert_ptr_ne(res, NULL);
    ck_assert_str_eq(
        res->post_board->fen,
        "2kr2r1/p1pp1ppp/8/8/1p6/P1P1p3/1P1PPPPP/RNBQKBNR w - - - 2");

    res = apply_moves_to_fen(
        "r3k2r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b kq - - -",
        3, ks_rook_ks_castle);
    ck_assert_ptr_eq(res, NULL);
}
END_TEST

START_TEST(test_en_passant)
{
    struct move *res;
    char *valid_moves[2] = {"c5", "dxc6"};
    char *invalid_moves[4] = {"c5", "a3", "a5", "dxc6"};

    res = apply_moves_to_fen(
        "3k4/p1p5/8/3P4/8/8/P7/3K4 b - - - -", 2, valid_moves);
    ck_assert_ptr_ne(res, NULL);
    ck_assert_str_eq(
        res->post_board->fen,
        "3k4/p7/2P5/8/8/8/P7/3K4 b - - - 1");

    res = apply_moves_to_fen(
        "3k4/p1p5/8/3P4/8/8/P7/3K4 b - - - -", 4, invalid_moves);
    ck_assert_ptr_eq(res, NULL);
}
END_TEST

START_TEST(test_not_in_check)
{
    ck_assert_int_eq(
        MOVE_AVAILABLE,
        check_status("3k4/2p5/8/B2P4/8/8/8/3K4 b - - - -"));
}
END_TEST

START_TEST(test_in_check)
{
    ck_assert_int_eq(
        CHECK,
        check_status("3k4/8/8/B2P4/8/8/8/3K4 b - - - -"));
    ck_assert_int_eq(
        CHECK,
        check_status("3k4/8/8/B2P4/8/8/8/3K1r2 b - - - -"));
    ck_assert_int_eq(
        CHECK,
        check_status("3k4/8/8/B2P4/8/8/8/3K1r2 w - - - -"));
    ck_assert_int_eq(
        CHECK,
        check_status("3k4/3Q4/8/B2P4/8/8/8/3K4 b - - - -"));
    ck_assert_int_eq(
        CHECK,
        check_status("3k4/r2Q4/8/3P4/B7/8/8/3K4 b - - - -"));
    ck_assert_int_eq(
        CHECK,
        check_status("2Bkr3/r7/3Q4/8/B7/8/8/3K4 b - - - -"));
    ck_assert_int_eq(
        CHECK,
        check_status("k7/8/3r4/4q3/5r2/8/2R5/4K3 w - - - -"));
}
END_TEST

START_TEST(test_check_movement)
{
    struct move *res;
    char *qs_castle[1] = {"O-O-O"};
    char *move_into_check[1] = {"f6"};
    res = apply_moves_to_fen(
        "r3kbnr/p1p2ppp/8/8/3R4/P7/1PPPPPPP/RNBQKBNR b kq - - -",
        1, qs_castle);
    ck_assert_ptr_eq(res, NULL);

    res = apply_moves_to_fen(
        "5bnr/4p1pq/4Qpkr/7p/7P/4P3/PPPP1PP1/RNB1KBNR b - - - -",
        1, move_into_check);
    ck_assert_ptr_eq(res, NULL);
}
END_TEST

START_TEST(test_checkmate)
{
    ck_assert_int_eq(
        CHECKMATE,
        check_status("3k4/3Q4/8/3P4/B7/8/8/3K4 b - - - -"));
}
END_TEST

START_TEST(test_stalemate)
{
    ck_assert_int_eq(
        STALEMATE,
        check_status("3k4/8/8/8/8/1r6/2r5/K7 w - - - -"));
    ck_assert_int_eq(
        STALEMATE,
        check_status("5bnr/4p1pq/4Qpkr/7p/7P/4P3/PPPP1PP1/RNB1KBNR b - - - -"));
}
END_TEST

int main()
{
    int n_failures;
    Suite *s;
    SRunner *sr;
    TCase *tc;

    s = suite_create("grandmaster");
    tc = tcase_create("movement");
    tcase_add_test(tc, test_pawn);
    tcase_add_test(tc, test_bishop);
    tcase_add_test(tc, test_castle);
    tcase_add_test(tc, test_en_passant);
    suite_add_tcase(s, tc);


    tc = tcase_create("in_check");
    tcase_add_test(tc, test_in_check);
    tcase_add_test(tc, test_not_in_check);
    tcase_add_test(tc, test_checkmate);
    tcase_add_test(tc, test_stalemate);
    tcase_add_test(tc, test_check_movement);
    suite_add_tcase(s, tc);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    n_failures = srunner_ntests_failed(sr);
    srunner_free(sr);

    return n_failures;
}
