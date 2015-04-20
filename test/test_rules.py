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

move_parser = "testbin/move_parser"
verifier = "testbin/test_rules_harness"

class RulesTest(unittest.TestCase):
    def setUp(self):
        if not os.path.exists(move_parser):
            raise Exception("run tests by running `make test`")

    def ensure_valid(self, *move_list):
        args = [move_parser]
        args.extend(move_list)
        res = subprocess.check_output(args)
        moves = json.loads(res)
        self.assertEqual(len(move_list), len(moves))
        return moves

    def ensure_invalid(self, *move_list):
        args = [move_parser]
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
            args = [verifier, start, start]
        else:
            args = [verifier, start, expected]
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
        if proc.returncode != expect_code:
            print "\nFAILURE -------------\n%s\n^^^^^^^^^^^^^^^^^^^^^" % stdout
            self.fail("expected return code %s, got %s"
                      % (expect_code, proc.returncode))

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

        self.ensure_result(start, kend, "Rb8", "c3", "O-O")
        # invalid; can't castle queenside after queenside rook has moved
        self.ensure_result(start, None, "Rb8", "c3", "O-O-O")

        self.ensure_result(start, qend, "Rg8", "c3", "O-O-O")
        # invalid; can't castle kingside after kingside rook has moved
        self.ensure_result(start, None, "Rg8", "c3", "O-O")
# 
#     def testBadCastle(self):
#         board = """
#         bR __ __ __ bK bB bN bR
#         bp __ bp bp __ bp bp bp
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ bp __ __ __ __ __ __
#         wp __ __ __ bp __ __ __
#         __ wp wp wp wp wp wp wp
#         wR wN wB wQ wK wB wN wR
#         """
#         b = chess.Board.parse(board)
#         self.assertTrue(b.can_castle(chess.black, chess.queenside))
#         self.assertTrue(b.can_castle(chess.black, chess.kingside))
#         m = chess.Move.on_board((7, 0), (7, 2), b)
#         b = b.apply(m)
#         self.assertFalse(b.can_castle(chess.black, chess.queenside))
#         self.assertTrue(b.can_castle(chess.black, chess.kingside))
# 
#         board = """
#         bR __ __ __ bK bB bN bR
#         bp __ bp __ __ bp bp bp
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wR __ __ __ __
#         wp __ __ __ bp __ __ __
#         __ wp wp wp wp wp wp wp
#         wR wN wB wQ wK wB wN wR
#         """
#         b = chess.Board.parse(board)
#         m = chess.Move.on_board((7, 4), (7, 2), b)
#         self.assertFalse(m.is_valid(b))
# 
#     def testEnPassant(self):
#         board = """
#         __ __ __ bK __ __ __ __
#         __ __ bp __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wp __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         m = chess.Move.on_board((6, 2), (4, 2), b)
#         self.assertTrue(m.is_valid(b))
#         b = b.apply(m)
#         m = chess.Move.on_board((4, 3), (5, 2), b)
#         self.assertTrue(m.is_valid(b))
#         b = b.apply(m)
#         self.assertFalse(list(b.find(chess.Piece(chess.black, chess.pawn))))
# 
#     def testInCheck(self):
#         board = """
#         __ __ __ bK __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         wB __ __ wp __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertTrue(chess.in_check(b, chess.black))
# 
#         board = """
#         __ __ __ bK __ __ __ __
#         __ __ bp __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         wB __ __ wp __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertFalse(chess.in_check(b, chess.black))
# 
#         board = """
#         __ __ __ __ __ __ __ __
#         __ __ __ bK __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wp __ __ __ __
#         wB __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertTrue(chess.in_check(b, chess.black))
# 
#     def testBadInCheck(self):
#         board = """
#         __ __ __ __ __ __ __ __
#         __ __ __ bK __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wp __ __ __ __
#         wB __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ bR __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertTrue(chess.in_check(b, chess.black))
#         self.assertTrue(chess.in_check(b, chess.white))
# 
#     def testBadCheck(self):
#         board = """
#         __ __ __ __ __ __ __ __
#         __ __ __ bK __ bR __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wp __ __ __ __
#         wB __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         m = chess.Move.on_board((6, 5), (0, 5), b)
#         self.assertFalse(m.is_valid(b))
# 
#     def testCheckmate(self):
#         board = """
#         __ __ __ bK __ __ __ __
#         __ __ __ wQ __ __ __ __
#         __ __ __ __ __ __ __ __
#         wB __ __ wp __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         # Kxd7 is a valid move.
#         self.assertFalse(chess.in_checkmate(b, chess.black))
# 
#         board = """
#         __ __ __ bK __ __ __ __
#         __ __ __ wQ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wp __ __ __ __
#         wB __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertTrue(chess.in_checkmate(b, chess.black))
# 
#         board = """
#         __ __ __ bK __ __ __ __
#         bR __ __ wQ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wp __ __ __ __
#         wB __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertFalse(chess.in_checkmate(b, chess.black))
# 
#         board = """
#         __ __ bB bK bR __ __ __
#         bR __ __ __ __ __ __ __
#         __ __ __ wQ __ __ __ __
#         __ __ __ wp __ __ __ __
#         wB __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertFalse(chess.in_checkmate(b, chess.black))
# 
#     def testStalemate(self):
#         board = """
#         __ __ __ bK __ __ __ __
#         __ __ __ wQ __ __ __ __
#         __ __ __ __ __ __ __ __
#         wB __ __ wp __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertFalse(chess.in_stalemate(b, chess.black))
# 
#         board = """
#         __ __ __ bK __ __ __ __
#         __ __ __ wQ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wp __ __ __ __
#         wB __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertFalse(chess.in_stalemate(b, chess.black))
# 
#         board = """
#         bK __ __ __ __ __ __ __
#         __ __ wR __ __ __ __ __
#         __ wR __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ __ __ __ __ __
#         __ __ __ wK __ __ __ __
#         """
#         b = chess.Board.parse(board)
#         self.assertTrue(chess.in_stalemate(b, chess.black))


if __name__ == '__main__':
    unittest.main()
