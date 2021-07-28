// RUN: AArch64
// FUNC-DECL: int test()
// TEST-CASE: test() -> 5

#include "tetris.h"

#define BOARD_HEIGHT 20
#define BOARD_WIDTH 10

#define ABS(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

const unsigned char tetrominos[7][16] = {
    {0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0}};


Tetris new_tetris() {
  Tetris tetris;

  for (int row = 0; row < BOARD_HEIGHT; row++)
    for (int col = 0; col < BOARD_WIDTH; col++)
      tetris.board[row][col] = 0;

  tetris.score = 0;
  tetris.curr_piece = (Piece){.kind = 0, .x = 0, .y = 5, .rotation = 0};
  tetris.next_piece_kind = 0;
  tetris.bot_target_piece =
      (Piece){.kind = 100, .x = 100, .y = 100, .rotation = 100};

  return tetris;
}

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

void update_current_piece(Tetris *self, unsigned char value) {
  for (uint row = 0; row < 4; row++)
    for (uint col = 0; col < 4; col++) {
      const uint rotated_idx = rotate(row, col, self->curr_piece.rotation);

      if (tetrominos[self->curr_piece.kind][rotated_idx] == 1)
        self->board[row + self->curr_piece.x][col + self->curr_piece.y] = value;
    }
}

void add_current_piece(Tetris *self) {
  update_current_piece(self, self->curr_piece.kind + 1);
}

void remove_current_piece(Tetris *self) { update_current_piece(self, 0); }

uint max_rotation(uint kind) {
  uint rotation_degree = 4;

  switch (kind) {
  case 0:
  case 2:
  case 6:
    rotation_degree = 2;
    break;

  case 4:
    rotation_degree = 1;
    break;

  default:
    break;
  }

  return rotation_degree;
}

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

ull board_max_height(const Tetris *self) {
  for (uint row = 0; row < BOARD_HEIGHT; row++)
    for (uint col = 0; col < BOARD_WIDTH; col++)
      if (self->board[row][col] != 0)
        return BOARD_HEIGHT - row;

  return 0;
}

ull board_hole_factor(const Tetris *self) {
  ull hole_factor = 0;

  for (uint col = 0; col < BOARD_WIDTH; col++) {
    uint row = 0;

    // find first non empty block
    while (row < BOARD_HEIGHT && self->board[row][col] == 0)
      row++;

    // if no such block found then continue with the next column
    if (row == BOARD_HEIGHT)
      continue;

    // otherwise count the holes weighted by its depth
    for (uint r = row; r < BOARD_HEIGHT; r++)
      if (self->board[r][col] == 0)
        hole_factor += r;
  }

  return hole_factor;
}

ull board_line_continuity(const Tetris *self) {
  ull continuity_factor = 0;

  for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
    ull curr_line_con_from_left = 0;
    ull curr_line_con_from_right = 0;
    uint col = 0;

    while (col < BOARD_WIDTH && self->board[row][col++] != 0)
      curr_line_con_from_left++;

    col = BOARD_WIDTH;
    while (col > 0 && self->board[row][col-- - 1] != 0)
      curr_line_con_from_right++;

    const uint factor = MAX(curr_line_con_from_left, curr_line_con_from_right);
    continuity_factor += factor * factor * row;
  }

  return continuity_factor;
}

ull board_line_filledness(const Tetris *self) {
  ull filledness_factor = 0;

  for (uint row = 1; row < BOARD_HEIGHT; row++) {
    uint curr_line_filledness = 0;

    for (uint col = 0; col < BOARD_WIDTH; col++)
      if (self->board[row][col] != 0)
        curr_line_filledness++;

    filledness_factor += curr_line_filledness * curr_line_filledness * row;
  }

  return filledness_factor;
}

uint board_filled_lines(const Tetris *self) {
  uint filled_lines = 0;

  for (uint row = 0; row < BOARD_HEIGHT; row++) {
    bool filled = true;
    for (uint col = 0; col < BOARD_WIDTH; col++)
      if (self->board[row][col] == 0) {
        filled = false;
        break;
      }

    if (filled)
      filled_lines++;
  }

  return filled_lines;
}

ull fitness(const Tetris *self, uint *params) {
  ull filled_lines = board_filled_lines(self);
  filled_lines *= 10;
  filled_lines *= filled_lines;
  ull hole_fact = board_hole_factor(self);
  hole_fact *= hole_fact;
  ull bumpiness = board_bumpiness(self);
  ull max_height = board_max_height(self);
  ull continuity = board_line_continuity(self);
  ull filledness = board_line_filledness(self);

  ull fitness = 1ull << 50;
  fitness = MAX(fitness - params[0] * 100 * hole_fact, 0);
  fitness = MAX(fitness - params[1] * 1000 * bumpiness, 0);
  fitness = MAX(fitness - params[2] * 40 * max_height, 0);
  fitness += (params[3] * 4 * continuity) / 100;
  fitness += params[4] * 100 * filledness;
  fitness += params[5] * 1000 * filled_lines;

  return fitness;
}

Action bot(Tetris *self, uint *params) {
  const uint kind = self->curr_piece.kind;
  ull best_fitness = 0;
  Piece best_piece = self->curr_piece;
  const Piece original_piece = self->curr_piece;

  if (self->bot_target_piece.kind > 6) {
    for (uint rotation = 0; rotation < max_rotation(kind); rotation++)
      for (int col = -2; col < BOARD_WIDTH + 2; col++) {
        int curr_piece_x = self->curr_piece.x;

        if (!does_piece_fit(self, kind, rotation, curr_piece_x, col))
          continue;

        while (does_piece_fit(self, kind, rotation, curr_piece_x + 1, col))
          curr_piece_x++;

        self->curr_piece = (Piece){
            .kind = kind, .rotation = rotation, .x = curr_piece_x, .y = col};
        add_current_piece(self);

        uint kind_next = self->next_piece_kind;
        self->curr_piece =
            (Piece){.kind = kind_next, .rotation = 0, .x = 0, .y = 5};

        Piece original_next_piece = self->curr_piece;

        for (uint rotation_next = 0; rotation_next < max_rotation(kind_next);
             rotation_next++)
          for (int col_next = -2; col_next < BOARD_WIDTH + 2; col_next++) {
            int next_piece_x = self->curr_piece.x;

            if (!does_piece_fit(self, kind_next, rotation_next, next_piece_x,
                                col_next))
              continue;

            while (does_piece_fit(self, kind_next, rotation_next,
                                  next_piece_x + 1, col_next))
              next_piece_x++;

            self->curr_piece = (Piece){.kind = kind_next,
                .rotation = rotation_next,
                .x = next_piece_x,
                .y = col_next};
            add_current_piece(self);

            ull curr_fitness = fitness(self, params);

            remove_current_piece(self);
            self->curr_piece = original_next_piece;

            if (curr_fitness > best_fitness) {
              best_piece = (Piece){.kind = kind,
                  .rotation = rotation,
                  .x = curr_piece_x,
                  .y = col};
              best_fitness = curr_fitness;
            }
          }

        self->curr_piece = (Piece){
            .kind = kind, .rotation = rotation, .x = curr_piece_x, .y = col};

        remove_current_piece(self);
        self->curr_piece = original_piece;
      }

    self->bot_target_piece = best_piece;
  }

  Action bot_action = MOVE_DOWN;

  if (original_piece.rotation != self->bot_target_piece.rotation)
    bot_action = ROTATE;
  else if (original_piece.y != self->bot_target_piece.y) {
    if (original_piece.y < self->bot_target_piece.y)
      bot_action = MOVE_RIGHT;
    else
      bot_action = MOVE_LEFT;
  }

  return bot_action;
}

int test() {
  Tetris tetris;
  uint Params[6] = {1, 1, 1, 1, 1, 1};
  int result = 0;

  tetris = new_tetris();

  for (int i = 0; i < BOARD_WIDTH - 1; i++)
    tetris.board[19][i] = 1;

  result += bot(&tetris, Params); // 2

  tetris = new_tetris();

  for (int i = 2; i < BOARD_WIDTH; i++)
    tetris.board[19][i] = 1;

  result += bot(&tetris, Params); // 3

  return result;
}
