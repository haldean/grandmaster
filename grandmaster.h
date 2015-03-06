#ifndef __GRANDMASTER_H__
#define __GRANDMASTER_H__

#include <stdint.h>

typedef uint64_t player_id_t;

typedef enum {
    PAWN = 'p',
    ROOK = 'r',
    KNIGHT = 'n',
    BISHOP = 'b',
    QUEEN = 'q',
    KING = 'k',
} piece_type_t;
const piece_type_t ALL_PIECES[] = { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };

typedef enum {
    WHITE = 'w',
    BLACK = 'b',
} color_t;

#define opposite(x) ((x) == WHITE ? BLACK : WHITE)

struct piece {
    piece_type_t piece_type;
    color_t color;
};

struct position {
    uint8_t rank;
    uint8_t file;
};

struct move {
    struct position start;
    struct position end;
    color_t player;
    char *algebraic;
    const struct move *parent;
    struct board *post_board;
};

struct board {
    struct piece board[8][8];
};

struct game {
    player_id_t player_white;
    player_id_t player_black;
    struct move *current;
};

/* Parse algebraic notation and return the result. **out is set to null if the
 * input was not a valid move. */
void
parse_algebraic(
    const char *notation,
    const struct move *last_move,
    struct move **out);

#endif
