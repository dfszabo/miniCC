// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 18

#include "tetris.h"

#define BOARD_HEIGHT 20
#define BOARD_WIDTH 10

#define ABS(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))

ull board_bumpiness(const Tetris *self) {
  ull bumpiness_factor = 0;
  unsigned char heights[BOARD_WIDTH];

  for (uint i = 0; i < BOARD_WIDTH; i++)
    heights[i] = 0;

  for (uint col = 0; col < BOARD_WIDTH; col++)
    for (uint row = 0; row < BOARD_HEIGHT; row++)
      if (self->board[row][col] != 0) {
        heights[col] = BOARD_HEIGHT - row;
        break;
      }

  for (uint i = 0; i < BOARD_WIDTH - 1; i++)
    bumpiness_factor += ABS(heights[i], heights[i + 1]);

  return bumpiness_factor;
}

Tetris new_tetris() {
  Tetris tetris;

  for (int row = 0; row < BOARD_HEIGHT; row++)
    for (int col = 0; col < BOARD_WIDTH; col++)
      tetris.board[row][col] = 0;

  return tetris;
}

int test() {
  ull res = 0;
  Tetris tetris;

  tetris = new_tetris();

  for (int row = BOARD_HEIGHT - 1; row > 10; row--)
      tetris.board[row][2] = 1;

  res += board_bumpiness(&tetris); // 18

  return res;
}
