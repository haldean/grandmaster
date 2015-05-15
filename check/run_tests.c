/*
 * run_tests.c: runs all check test suites
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

#include "test_core.h"
#include "test_tree.h"


int main()
{
    int n_failures;
    SRunner *sr;

    sr = srunner_create(make_core_suite());
    srunner_add_suite(sr, make_tree_suite());
    srunner_run_all(sr, CK_NORMAL);
    n_failures = srunner_ntests_failed(sr);
    srunner_free(sr);

    return n_failures;
}
