// RUN: AArch64
// FUNC-DECL: unsigned test()
// TEST-CASE: test() -> 100

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

uint test_assign(Tetris *t) {
  t->curr_piece = (Piece){.kind = 2, .y = 4, .x = 3, .rotation = 5};
  int res = 1;
  res = res && t->curr_piece.kind == 2;
  res = res && t->curr_piece.x == 3;
  res = res && t->curr_piece.y == 4;
  res = res && t->curr_piece.rotation == 5;
  return res;
}

uint test() {
  Tetris tetris;
  uint y = 3;
  tetris.curr_piece = (Piece){.kind = 1, .x = 2, .y = y, .rotation = 4};
  return test_assign(&tetris) * 100;
}
