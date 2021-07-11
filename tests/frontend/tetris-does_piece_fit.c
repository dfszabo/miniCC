// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 3

#define BOARD_HEIGHT 20
#define BOARD_WIDTH 10
#define bool int
#define true 1
#define false 0

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

const unsigned char tetrominos[7][16] = {
    {0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0}};

uint rotate(uint x, uint y, uint rotation) {
  uint rotated_idx = 0;

  switch (rotation % 4) {
  case 0:
    rotated_idx = x * 4 + y;
    break;
  case 1:
    rotated_idx = 12 + x - (y * 4);
    break;
  case 2:
    rotated_idx = 15 - (x * 4) - y;
    break;
  case 3:
    rotated_idx = 3 - x + (y * 4);
    break;
  default:
    break;
  }

  return rotated_idx;
}

bool does_piece_fit(const Tetris *self, const uint kind, const uint rotation,
                    const uint x, const uint y) {
  for (uint tetromino_x = 0; tetromino_x < 4; tetromino_x++)
    for (uint tetromino_y = 0; tetromino_y < 4; tetromino_y++) {
      const uint rotated_idx = rotate(tetromino_x, tetromino_y, rotation);

      const uint absolute_x = x + tetromino_x;
      const uint absolute_y = y + tetromino_y;

      // if the absolute coordinates are valid board positions
      if (absolute_x >= 0 && absolute_x < BOARD_HEIGHT && absolute_y >= 0 &&
          absolute_y < BOARD_WIDTH) {
        // if both the tetromino and the board would overlap at this position
        // then it cannot fit
        if (tetrominos[kind][rotated_idx] != 0 &&
            self->board[absolute_x][absolute_y] != 0)
          return false;
      }
        // if the tetromino would be outside of the board boundary
      else if (tetrominos[kind][rotated_idx] != 0)
        return false;
    }
  return true;
}

Tetris new_tetris() {
  Tetris tetris;

  for (int row = 0; row < BOARD_HEIGHT; row++)
    for (int col = 0; col < BOARD_WIDTH; col++)
      tetris.board[row][col] = 0;

  return tetris;
}

int test() {
  int res = 0;
  Tetris tetris;

  tetris = new_tetris();

  res += does_piece_fit(&tetris, 6, 0, 0, 0); // 1
  res += does_piece_fit(&tetris, 6, 0, 0, 6); // 1
  res += does_piece_fit(&tetris, 6, 0, 10, 6); // 1
  res += does_piece_fit(&tetris, 6, 0, 0, -3); // 0
  res += does_piece_fit(&tetris, 6, 0, 20, 0); // 0

  return res;
}
