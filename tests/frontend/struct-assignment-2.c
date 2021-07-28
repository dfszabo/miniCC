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
  Piece p = t->curr_piece;
  int res = 1;
  res = res && p.kind == 1;
  res = res && p.x == 2;
  res = res && p.y == 3;
  res = res && p.rotation == 4;
  return res;
}

uint test() {
  Tetris tetris;
  uint y = 3;
  tetris.curr_piece = (Piece){.kind = 1, .x = 2, .y = y, .rotation = 4};
  return test_assign(&tetris) * 100;
}
