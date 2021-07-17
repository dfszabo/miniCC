//#ifndef TETRIS_H
#define TETRIS_H

//#include <stdbool.h>
#define bool int
#define true 1
#define false 0

typedef unsigned int uint;
typedef unsigned long long ull;

typedef struct piece {
  uint kind;
  int x;
  int y;
  uint rotation;
} Piece;

typedef struct tetris {
  unsigned char board[20][10];
  Piece curr_piece;
  uint next_piece_kind;
  uint score;
  Piece bot_target_piece;
} Tetris;

typedef enum { MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, ROTATE, QUIT, NONE } Action;

/// Function declarations

Tetris new_tetris();
uint rotate(uint x, uint y, uint rotation);
bool does_piece_fit(const Tetris *self, const uint kind, const uint rotation,
                    const uint x, const uint y);
void remove_filled_line(Tetris *self, uint row_index);
void find_and_remove_filled_lines(Tetris *self);
void update_current_piece(Tetris *self, unsigned char value);
void add_current_piece(Tetris *self);
void remove_current_piece(Tetris *self);
bool move_piece_down(Tetris *self);
bool move_piece_left(Tetris *self);
bool move_piece_right(Tetris *self);
bool rotate_piece(Tetris *self);

/// Bot functions
ull board_bumpiness(const Tetris *self);
ull board_max_height(const Tetris *self);
ull board_hole_factor(const Tetris *self);
ull board_line_continuity(const Tetris *self);
ull board_line_filledness(const Tetris *self);
uint board_filled_lines(const Tetris *self);
ull fitness(const Tetris *self, uint *params);
uint max_rotation(uint kind);
Action bot(Tetris *self, uint *params);

//#endif
