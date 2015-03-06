#include "grandmaster.h"

#include <stdlib.h>
#include <stdbool.h>

#define sign(x) (((x) > 0) - ((x) < 0))

bool
any_between(
    const struct position start,
    const struct position end,
    const struct board *board)
{
    int rank_step;
    int file_step;
    int d_rank;
    int d_file;
    const struct piece *p;

    rank_step = sign(end.rank - start.rank);
    file_step = sign(end.file - start.file);

    while (start.rank + d_rank != end.rank || start.file + d_file != end.file) {
        p = &board->board[start.rank + d_rank][start.file + d_file];
        if (p->piece_type != 0)
            return true;
        d_rank += rank_step;
        d_file += file_step;
    }
    return false;
}

bool
pawn_movement_valid(
    const struct move *move,
    const struct move *last_move)
{
    bool is_capture;
    struct board *b;
    struct piece *last_move_piece;
    int d_rank;

    if (any_between(move->start, move->end, last_move->post_board))
        return false;
    b = last_move->post_board;
    is_capture = b->board[move->end.rank][move->end.file].piece_type != 0;

    if (is_capture) {
        if (abs(move->start.file - move->end.file) != 1)
            return false;
        if (move->player == WHITE) {
            if (move->end.rank - move->start.rank != 1)
                return false;
        } else {
            if (move->end.rank - move->start.rank != -1)
                return false;
        }
    } else if (move->start.file != move->end.file) {
        /* this might be en passant. let's check it out. */
        /* we can only capture pawns that moved last turn. */
        if (move->end.file != last_move->end.file)
            return false;
        last_move_piece = &b->board[last_move->end.rank][last_move->end.file];
        if (last_move_piece->piece_type != PAWN)
            return false;
        if (move->player == WHITE) {
            if (move->start.rank != 4 || move->end.rank != 5)
                return false;
        } else {
            if (move->start.rank != 3 || move->end.rank != 2)
                return false;
        }
    } else {
        if (move->start.file != move->end.file)
            return false;
        d_rank = move->end.rank - move->start.rank;
        if (abs(d_rank) > 2)
            return false;
        if (move->player == WHITE) {
            if (d_rank < 0)
                return false;
            if (d_rank == 2 && move->start.rank != 1)
                return false;
        }
        if (move->player == WHITE) {
            if (d_rank > 0)
                return false;
            if (d_rank == -2 && move->start.rank != 6)
                return false;
        }
    }
    return true;
}

bool
rook_movement_valid(
    const struct move *move,
    const struct move *last_move)
{
    if (any_between(move->start, move->end, last_move->post_board))
        return false;
    return move->start.rank == move->end.rank
        || move->start.file == move->end.file;
}

bool
knight_movement_valid(
    const struct move *move,
    const struct move *last_move)
{
    int d_rank;
    int d_file;

    d_rank = abs(move->start.rank - move->end.rank);
    d_file = abs(move->start.file - move->end.file);
    return (d_rank == 2 && d_file == 1) || (d_rank == 1 && d_file == 2);
}

bool
bishop_movement_valid(
    const struct move *move,
    const struct move *last_move)
{
    int d_rank;
    int d_file;

    d_rank = abs(move->start.rank - move->end.rank);
    d_file = abs(move->start.file - move->end.file);
    if (d_rank != d_file)
        return false;
    if (any_between(move->start, move->end, last_move->post_board))
        return false;
    return true;
}

bool
queen_movement_valid(
    const struct move *move,
    const struct move *last_move)
{
    return bishop_movement_valid(move, last_move)
        || rook_movement_valid(move, last_move);
}

bool
king_movement_valid(
    const struct move *move,
    const struct move *last_move)
{
    int d_rank;
    int d_file;

    d_rank = abs(move->start.rank - move->end.rank);
    d_file = abs(move->start.file - move->end.file);
    return d_rank <= 1 && d_file <= 1;
}

bool
is_movement_valid(
    const struct move *move,
    const struct move *last_move)
{
    struct piece *piece;
    struct piece *captured;

    if (0 > move->start.rank || 7 < move->start.rank)
        return false;
    if (0 > move->start.file || 7 < move->start.file)
        return false;
    if (0 > move->end.rank || 7 < move->end.rank)
        return false;
    if (0 > move->end.file || 7 < move->end.file)
        return false;

    piece = &last_move->post_board->board[move->start.rank][move->start.file];
    if (piece->piece_type == 0 || piece->color == 0)
        return false;

    /* can't capture our own pieces. */
    captured = &last_move->post_board->board[move->end.rank][move->end.file];
    if (captured->color == piece->color)
        return false;

    switch (piece->piece_type) {
        case PAWN:
            return pawn_movement_valid(move, last_move);
        case ROOK:
            return rook_movement_valid(move, last_move);
        case KNIGHT:
            return knight_movement_valid(move, last_move);
        case BISHOP:
            return bishop_movement_valid(move, last_move);
        case QUEEN:
            return queen_movement_valid(move, last_move);
        case KING:
            return king_movement_valid(move, last_move);
    }
    return false;
}
