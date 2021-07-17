// RUN: AArch64
// FUNC-DECL: unsigned new_tetris()
// TEST-CASE: new_tetris() -> 5

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
