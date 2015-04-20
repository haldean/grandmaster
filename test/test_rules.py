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
        args = [verifier, start, start]
        proc = subprocess.Popen(args, stdin=subprocess.PIPE)
        proc.communicate(pgn_data)
        if expected is None:
            self.assertEqual(1, proc.returncode)
        else:
            self.assertEqual(0, proc.returncode)

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
# 
#     def testCastle(self):
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
#         m = chess.Move.on_board((7, 4), (7, 2), b)
#         self.assertTrue(m.is_valid(b))
#         m = chess.Move.on_board((7, 4), (7, 6), b)
#         self.assertFalse(m.is_valid(b))
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
