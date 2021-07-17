// RUN: AArch64
// FUNC-DECL: unsigned new_tetris()
// TEST-CASE: new_tetris() -> 5

typedef unsigned uint;

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

uint new_tetris() {
  Tetris tetris;
  uint y = 3;
  tetris.curr_piece = (Piece){.kind = 1, .x = 2, .y = y, .rotation = 4};
  return tetris.curr_piece.x + tetris.curr_piece.y;
}
