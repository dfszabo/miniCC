// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 1

#include "tetris.h"

#define BOARD_HEIGHT 20
#define BOARD_WIDTH 10

Tetris new_tetris() {
  Tetris tetris;

  for (int row = 0; row < BOARD_HEIGHT; row++)
    for (int col = 0; col < BOARD_WIDTH; col++)
      tetris.board[row][col] = 0;

  tetris.score = 0;
  tetris.curr_piece = (Piece){.kind = 0, .x = 0, .y = 5, .rotation = 0};
  tetris.next_piece_kind = 1;
  tetris.bot_target_piece =
      (Piece){.kind = 100, .x = 100, .y = 100, .rotation = 100};

  return tetris;
}

int test() {
  int res = 1;
  Tetris tetris;

  tetris = new_tetris();

  for (int row = 0; row < BOARD_HEIGHT; row++)
    for (int col = 0; col < BOARD_WIDTH; col++)
      res = res && tetris.board[row][col] == 0;

  res = res && tetris.score == 0;

  res = res && tetris.curr_piece.kind == 0;
  res = res && tetris.curr_piece.x == 0;
  res = res && tetris.curr_piece.y == 5;
  res = res && tetris.curr_piece.rotation == 0;

  res = res && tetris.next_piece_kind == 1;

  res = res && tetris.bot_target_piece.kind == 100;
  res = res && tetris.bot_target_piece.x == 100;
  res = res && tetris.bot_target_piece.y == 100;
  res = res && tetris.bot_target_piece.rotation == 100;

  return res;
}
