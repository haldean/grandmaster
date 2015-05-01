# test_rules.py: functional tests for move parsing
# Copyright (C) 2015, Haldean Brown
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import json
import os
import subprocess
import unittest

move_parser_bin = "testbin/move_parser"
verifier_bin = "testbin/test_rules_harness"
is_in_check_bin = "testbin/is_in_check"

check = 0
no_check = 1
checkmate = 2
stalemate = 3
check_names = {
    check: "check",
    no_check: "no_check",
    checkmate: "checkmate",
    stalemate: "stalemate",
}

class RulesTest(unittest.TestCase):
    def setUp(self):
        if not os.path.exists(move_parser_bin):
            raise Exception("run tests by running `make test`")
        if not os.path.exists(verifier_bin):
            raise Exception("run tests by running `make test`")
        if not os.path.exists(is_in_check_bin):
            raise Exception("run tests by running `make test`")

    def ensure_valid(self, *move_list):
        args = [move_parser_bin]
        args.extend(move_list)
        res = subprocess.check_output(args)
        moves = json.loads(res)
        self.assertEqual(len(move_list), len(moves))
        return moves

    def ensure_invalid(self, *move_list):
        args = [move_parser_bin]
        args.extend(move_list)
        try:
            res = subprocess.check_output(args)
            self.fail("move list %s did not fail" % move_list)
        except subprocess.CalledProcessError:
            pass

    def ensure_result(self, start, expected, *move_list):
        pairs = [tuple(move_list[i:i+2]) for i in range(0, len(move_list), 2)]
        pgn_data = " ".join(
            "%d.%s %s" % (i + 1, p[0], p[1]) if len(p) == 2
                else "%d.%s" % (i + 1, p[0])
            for i, p in enumerate(pairs))
        if expected is None:
            args = [verifier_bin, start, start]
        else:
            args = [verifier_bin, start, expected]
        proc = subprocess.Popen(
            args, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT)
        stdout, _ = proc.communicate(pgn_data)
        # retcode == 0 means all is good, retcode == 1 means that the moves
        # didn't parse, and retcode == 2 means that the moves parsed but the
        # result didn't match.
        if expected is None:
            expect_code = 1
        else:
            expect_code = 0
        print stdout.strip()
        self.assertEqual(proc.returncode, expect_code)

    def ensure_in_check(self, fen, in_check):
        args = [is_in_check_bin, fen]
        proc = subprocess.Popen(
            args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        stdout, _ = proc.communicate()
        stdout = stdout.strip(" \n")
        self.assertTrue(
            proc.returncode in check_names,
            "expected %s, got return code %s: %s\nstdout:\n%s"
            % (check_names[in_check], proc.returncode, fen, stdout))
        self.assertEqual(
            in_check, proc.returncode,
            "expected %s, got %s: %s\nstdout:\n%s"
            % (check_names[in_check], check_names[proc.returncode], fen, stdout))

    def testPawn(self):
        self.ensure_valid("a4")
        self.ensure_valid("a3")
        self.ensure_invalid("a5")

        self.ensure_valid("a4", "b5")
        self.ensure_valid("a4", "b5", "axb5")
        self.ensure_invalid("a4", "c5", "axb5")

    def testBishop(self):
        self.ensure_valid("a4", "b5", "a5", "Ba6")
        self.ensure_invalid("a4", "b5", "a5", "b6", "a6", "Ba6")

    def testCastle(self):
        start = "r3kbnr/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b q - - -"
        end   = "2kr1bnr/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR w - - - -"
        self.ensure_result(start, end, "O-O-O")

        start = "r3kbnr/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b - - - -"
        self.ensure_result(start, None, "O-O-O")

    def testBadCastle(self):
        start = "r3k2r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR b kq - - -"
        qend  = "2kr3r/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR w - - - -"
        kend  = "r4rk1/p1pp1ppp/8/8/1p6/P3p3/1PPPPPPP/RNBQKBNR w - - - -"
        self.ensure_result(start, qend, "O-O-O")
        self.ensure_result(start, kend, "O-O")

        kend  = "1r3rk1/p1pp1ppp/8/8/1p6/P1P1p3/1P1PPPPP/RNBQKBNR w - - - -"
        self.ensure_result(start, kend, "Rb8", "c3", "O-O")
        # invalid; can't castle queenside after queenside rook has moved
        self.ensure_result(start, None, "Rb8", "c3", "O-O-O")

        qend  = "2kr2r1/p1pp1ppp/8/8/1p6/P1P1p3/1P1PPPPP/RNBQKBNR w - - - -"
        self.ensure_result(start, qend, "Rg8", "c3", "O-O-O")
        # invalid; can't castle kingside after kingside rook has moved
        self.ensure_result(start, None, "Rg8", "c3", "O-O")

    def testCastleThroughCheck(self):
        start = "r3kbnr/p1p2ppp/8/8/3R4/P7/1PPPPPPP/RNBQKBNR b kq - - -"
        self.ensure_result(start, None, "O-O-O")

    def testEnPassant(self):
        start = "3k4/2p5/8/3P4/8/8/8/3K4 b - - - -"
        end   = "3k4/8/2P5/8/8/8/8/3K4 b - - - -"
        self.ensure_result(start, end, "c5", "dxc6")

    def testInCheck(self):
        fen = "3k4/8/8/B2P4/8/8/8/3K4 b - - - -"
        self.ensure_in_check(fen, check)

        fen = "3k4/2p5/8/B2P4/8/8/8/3K4 b - - - -"
        self.ensure_in_check(fen, no_check)

        fen = "3k4/8/8/B2P4/8/8/8/3K1r2 b - - - -"
        self.ensure_in_check(fen, check)

        fen = "3k4/8/8/B2P4/8/8/8/3K1r2 w - - - -"
        self.ensure_in_check(fen, check)

    def testCheckmate(self):
        fen = "3k4/3Q4/8/B2P4/8/8/8/3K4 b - - - -"
        # Kxd7 is a valid move.
        self.ensure_in_check(fen, check)

        fen = "3k4/3Q4/8/3P4/B7/8/8/3K4 b - - - -"
        self.ensure_in_check(fen, checkmate)

        fen = "3k4/r2Q4/8/3P4/B7/8/8/3K4 b - - - -"
        # Rxd7 is a valid move.
        self.ensure_in_check(fen, check)

        fen = "2Bkr3/r7/3Q4/8/B7/8/8/3K4 b - - - -"
        # Rd7 is a valid move.
        self.ensure_in_check(fen, check)

        fen = "k7/8/3r4/4q3/5r2/8/2R5/4K3 w - - - -"
        # Re2 is a valid move.
        self.ensure_in_check(fen, check)

    def testStalemate(self):
        fen = "3k4/8/8/8/8/1r6/2r5/K7 w - - - -"
        self.ensure_in_check(fen, stalemate)


if __name__ == '__main__':
    unittest.main()
