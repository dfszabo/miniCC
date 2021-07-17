// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 40

#include "tetris.h"

#define BOARD_HEIGHT 20
#define BOARD_WIDTH 10

void remove_filled_line(Tetris *self, uint row_index) {
  for (uint row = row_index; row > 0; row--)
    for (uint col = 0; col < BOARD_WIDTH; col++)
      self->board[row][col] = self->board[row - 1][col];
}

void find_and_remove_filled_lines(Tetris *self) {
  uint found_lines = 0;
  int found_lines_indices[4] = {-1, -1, -1, -1};

  // searching the current tetromino piece lines from the bottom
  for (int x = 3; x >= 0; x--) {
    const uint row = x + self->curr_piece.x;

    // if outside the board, then skip iteration
    if (row >= BOARD_HEIGHT)
      continue;

    bool found = true;
    for (uint col = 0; col < BOARD_WIDTH; col++)
      if (self->board[row][col] == 0) {
        found = false;
        break;
      }

    // if filled line were found save its index and increase the counter
    if (found)
      found_lines_indices[found_lines++] = row;
  }

  for (uint i = 0; i < found_lines; i++) {
    remove_filled_line(self, found_lines_indices[i] + i);
  }

  // update score
  self->score += found_lines * 10;
}

Tetris new_tetris() {
  Tetris tetris;

  for (int row = 0; row < BOARD_HEIGHT - 3; row++)
    for (int col = 0; col < BOARD_WIDTH; col++)
      tetris.board[row][col] = 0;

  for (int row = BOARD_HEIGHT - 4; row < BOARD_HEIGHT; row++)
    for (int col = 0; col < BOARD_WIDTH; col++)
      tetris.board[row][col] = 1;

  tetris.score = 0;
  tetris.curr_piece.x = 16;

  return tetris;
}

int test() {
  Tetris tetris;

  tetris = new_tetris();

  find_and_remove_filled_lines(&tetris);

  return tetris.score;
}
