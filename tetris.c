#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "tetris.h"

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

/* 배열 정의 부분 */
tetris_location TETROMINOS[NUM_TETROMINOS][NUM_ORIENTATIONS][TETRIS] = {
  // I
  {{{1, 0}, {1, 1}, {1, 2}, {1, 3}},
   {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
   {{3, 0}, {3, 1}, {3, 2}, {3, 3}},
   {{0, 1}, {1, 1}, {2, 1}, {3, 1}}},
  // J
  {{{0, 0}, {1, 0}, {1, 1}, {1, 2}},
   {{0, 1}, {0, 2}, {1, 1}, {2, 1}},
   {{1, 0}, {1, 1}, {1, 2}, {2, 2}},
   {{0, 1}, {1, 1}, {2, 0}, {2, 1}}},
  // L
  {{{0, 2}, {1, 0}, {1, 1}, {1, 2}},
   {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
   {{1, 0}, {1, 1}, {1, 2}, {2, 0}},
   {{0, 0}, {0, 1}, {1, 1}, {2, 1}}},
  // O
  {{{0, 1}, {0, 2}, {1, 1}, {1, 2}},
   {{0, 1}, {0, 2}, {1, 1}, {1, 2}},
   {{0, 1}, {0, 2}, {1, 1}, {1, 2}},
   {{0, 1}, {0, 2}, {1, 1}, {1, 2}}},
  // S
  {{{0, 1}, {0, 2}, {1, 0}, {1, 1}},
   {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
   {{1, 1}, {1, 2}, {2, 0}, {2, 1}},
   {{0, 0}, {1, 0}, {1, 1}, {2, 1}}},
  // T
  {{{0, 1}, {1, 0}, {1, 1}, {1, 2}},
   {{0, 1}, {1, 1}, {1, 2}, {2, 1}},
   {{1, 0}, {1, 1}, {1, 2}, {2, 1}},
   {{0, 1}, {1, 0}, {1, 1}, {2, 1}}},
  // Z
  {{{0, 0}, {0, 1}, {1, 1}, {1, 2}},
   {{0, 2}, {1, 1}, {1, 2}, {2, 1}},
   {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
   {{0, 1}, {1, 0}, {1, 1}, {2, 0}}},
};

int GRAVITY_LEVEL[MAX_LEVEL+1] = {
// 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
  50, 48, 46, 44, 42, 40, 38, 36, 34, 32,
//10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  30, 28, 26, 24, 22, 20, 16, 12,  8,  4
};

/* 주어진 행과 열에 블록을 반납함 */
char tg_get(tetris_game *obj, int row, int column) {
  return obj->board[obj->cols * row + column];
}

/* 주어진 행과 열에 블록을 설정함 */
static void tg_set(tetris_game *obj, int row, int column, char value) {
  obj->board[obj->cols * row + column] = value;
}

/* 행과 열이 경계에 있는지 확인함 */
bool tg_check(tetris_game *obj, int row, int col) {
  return 0 <= row && row < obj->rows && 0 <= col && col < obj->cols;
}

/* 판 위에 블록을 놓는다. */
static void tg_put(tetris_game *obj, tetris_block block) {
  int i;
  for (i = 0; i < TETRIS; i++) {
    tetris_location cell = TETROMINOS[block.typ][block.ori][i];
    tg_set(obj, block.loc.row + cell.row, block.loc.col + cell.col,
           TYPE_TO_CELL(block.typ));
  }
}

/* 보드에서 블록 한줄을 지운다. */
static void tg_remove(tetris_game *obj, tetris_block block) {
  int i;
  for (i = 0; i < TETRIS; i++) {
    tetris_location cell = TETROMINOS[block.typ][block.ori][i];
    tg_set(obj, block.loc.row + cell.row, block.loc.col + cell.col, TC_EMPTY);
  }
}

/* 보드에서 블록을 배치할 수 있는지 확인 */
static bool tg_fits(tetris_game *obj, tetris_block block) {
  int i, r, c;
  for (i = 0; i < TETRIS; i++) {
    tetris_location cell = TETROMINOS[block.typ][block.ori][i];
    r = block.loc.row + cell.row;
    c = block.loc.col + cell.col;
    if (!tg_check(obj, r, c) || TC_IS_FILLED(tg_get(obj, r, c))) {
      return false;
    }
  }
  return true;
}

/* 임의의 테트리스 블럭 유형을 반환 */
static int random_tetromino(void) {
  return rand() % NUM_TETROMINOS;
}

/* 새롭게 하강하는 블록을 만들고 다음 하강하는 블록을 임의의 블록으로 채운다. */
static void tg_new_falling(tetris_game *obj) {
  // 떨어지는 테트리스블록을 넣는다.
  obj->falling = obj->next;
  obj->next.typ = random_tetromino();
  obj->next.ori = 0;
  obj->next.loc.row = 0;
  obj->next.loc.col = obj->cols/2 - 2;
}

/* 블록을 아래로 움직이게함 */
static void tg_do_gravity_tick(tetris_game *obj){
  obj->ticks_till_gravity--;
  if (obj->ticks_till_gravity <= 0) {
    tg_remove(obj, obj->falling);
    obj->falling.loc.row++;
    if (tg_fits(obj, obj->falling)) {
      obj->ticks_till_gravity = GRAVITY_LEVEL[obj->level];
    } else {
      obj->falling.loc.row--;
      tg_put(obj, obj->falling);

      tg_new_falling(obj);
    }
    tg_put(obj, obj->falling);
  }
}

/* 낙하하는 테트리스 블록을 왼쪽(1) 또는 오른쪽(+1)으로 이동합니다. */
static void tg_move(tetris_game *obj, int direction) {
  tg_remove(obj, obj->falling);
  obj->falling.loc.col += direction;
  if (!tg_fits(obj, obj->falling)) {
    obj->falling.loc.col -= direction;
  }
  tg_put(obj, obj->falling);
}

/* 아래에 쓰러져 있는 사각형 블록을 송부합니다. */
static void tg_down(tetris_game *obj) {
  tg_remove(obj, obj->falling);
  while (tg_fits(obj, obj->falling)) {
    obj->falling.loc.row++;
  }
  obj->falling.loc.row--;
  tg_put(obj, obj->falling);
  tg_new_falling(obj);
}

/* 낙하하는 블록을 원하는 방향(+/-1)으로 돌립니다. */
static void tg_rotate(tetris_game *obj, int direction) {
  tg_remove(obj, obj->falling);

  while (true) {
    obj->falling.ori = (obj->falling.ori + direction) % NUM_ORIENTATIONS;

    // 새로운 방향이 맞다면 if문을 빠져나옴
    if (tg_fits(obj, obj->falling))
      break;

    // 그렇지 않으면 왼쪽을 움직여 모양에 적합하게 만듬.
    obj->falling.loc.col--;
    if (tg_fits(obj, obj->falling))
      break;

    obj->falling.loc.col += 2;
    if (tg_fits(obj, obj->falling))
      break;

    // 원래의 위치로 되돌리고, 다음의 방향을 시험한다.
    obj->falling.loc.col--;
    // 최악의 경우는 원래 방향으로 돌아가고 루프가 종료됨
  }

tg_put(obj, obj->falling);
}

/* 낙하하는 블록을 홀드 버퍼에 있는 블록과 스왑하는 기능 */
static void tg_hold(tetris_game *obj) {
  tg_remove(obj, obj->falling);
  if (obj->stored.typ == -1) {
    obj->stored = obj->falling;
    tg_new_falling(obj);
  } else {
    int typ = obj->falling.typ, ori = obj->falling.ori;
    obj->falling.typ = obj->stored.typ;
    obj->falling.ori = obj->stored.ori;
    obj->stored.typ = typ;
    obj->stored.ori = ori;
    while (!tg_fits(obj, obj->falling)) {
      obj->falling.loc.row--;
    }
  }
  tg_put(obj, obj->falling);
}

/* 이동에 의해 지정된 액션을 실행. */
static void tg_handle_move(tetris_game *obj, tetris_move move) {
  switch (move) {
  case TM_LEFT:
    tg_move(obj, -1);
    break;
  case TM_RIGHT:
    tg_move(obj, 1);
    break;
  case TM_DROP:
    tg_down(obj);
    break;
  case TM_CLOCK:
    tg_rotate(obj, 1);
    break;
  case TM_COUNTER:
    tg_rotate(obj, -1);
    break;
  case TM_HOLD:
    tg_hold(obj);
    break;
  default:
    break;
  }
}

/* i값이 꽉찰 시 true로 리턴. */
static bool tg_line_full(tetris_game *obj, int i) {
  int j;
  for (j = 0; j < obj->cols; j++) {
    if (TC_IS_EMPTY(tg_get(obj, i, j)))
      return false;
  }
  return true;
}

/* 각 행을 r 위의 1개 아래로 이동합니다. */
static void tg_shift_lines(tetris_game *obj, int r) {
  int i, j;
  for (i = r-1; i >= 0; i--) {
    for (j = 0; j < obj->cols; j++) {
      tg_set(obj, i+1, j, tg_get(obj, i, j));
      tg_set(obj, i, j, TC_EMPTY);
    }
  }
}

/* 입력한 행을 찾고, 삭제하고, 이동하고 나서 클리어 한 줄수를 되돌립니다. */
static int tg_check_lines(tetris_game *obj) {
  int i, nlines = 0;
  tg_remove(obj, obj->falling); // don't want to mess up falling block

  for (i = obj->rows-1; i >= 0; i--) {
    if (tg_line_full(obj, i)) {
      tg_shift_lines(obj, i);
      i++; // do this line over again since they're shifted
      nlines++;
    }
  }

  tg_put(obj, obj->falling); // replace
  return nlines;
}

/* 클리어한 라인수를 고려하여 게임점수 조정 */
static void tg_adjust_score(tetris_game *obj, int lines_cleared) {
  static int line_multiplier[] = {0, 40, 100, 300, 1200};
  obj->points += line_multiplier[lines_cleared] * (obj->level + 1);
  if (lines_cleared >= obj->lines_remaining) {
    obj->level = MIN(MAX_LEVEL, obj->level + 1);
    lines_cleared -= obj->lines_remaining;
    obj->lines_remaining = LINES_PER_LEVEL - lines_cleared;
  } else {
    obj->lines_remaining -= lines_cleared;
  }
}

/* 게임이 끝나면 true 반환 */
static bool tg_game_over(tetris_game *obj) {
  int i, j;
  bool over = false;
  tg_remove(obj, obj->falling);
  for (i = 0; i < 2; i++) {
    for (j = 0; j < obj->cols; j++) {
      if (TC_IS_FILLED(tg_get(obj, i, j))) {
        over = true;
      }
    }
  }
  tg_put(obj, obj->falling);
  return over;
}

/* 주요기능 */
/*게임을 한번 합니다.프로세스의 중력(gravity), 사용자 입력(input), 스코어입니다. 아직 게임이 실행되어 있는 경우는 true, 종료된 경우는 false 반환*/
bool tg_tick(tetris_game *obj, tetris_move move) {
  int lines_cleared;
  // Handle gravity.
  tg_do_gravity_tick(obj);

  // Handle input.
  tg_handle_move(obj, move);

  // Check for cleared lines
  lines_cleared = tg_check_lines(obj);

  tg_adjust_score(obj, lines_cleared);

  // Return whether the game will continue (NOT whether it's over)
  return !tg_game_over(obj);
}

void tg_init(tetris_game *obj, int rows, int cols) {
  // Initialization logic
  obj->rows = rows;
  obj->cols = cols;
  obj->board = malloc(rows * cols);
  memset(obj->board, TC_EMPTY, rows * cols);
  obj->points = 0;
  obj->level = 0;
  obj->ticks_till_gravity = GRAVITY_LEVEL[obj->level];
  obj->lines_remaining = LINES_PER_LEVEL;
  srand(time(NULL));
  tg_new_falling(obj);
  tg_new_falling(obj);
  obj->stored.typ = -1;
  obj->stored.ori = 0;
  obj->stored.loc.row = 0;
  obj->next.loc.col = obj->cols/2 - 2;
  printf("%d", obj->falling.loc.col);
}

tetris_game *tg_create(int rows, int cols) {
  tetris_game *obj = malloc(sizeof(tetris_game));
  tg_init(obj, rows, cols);
  return obj;
}

void tg_destroy(tetris_game *obj) {
  // Cleanup logic
  free(obj->board);
}

void tg_delete(tetris_game *obj) {
  tg_destroy(obj);
  free(obj);
}

/* 파일에서 게임 불러오기(Load) 기능 */
tetris_game *tg_load(FILE *f) {
  tetris_game *obj = malloc(sizeof(tetris_game));
  fread(obj, sizeof(tetris_game), 1, f);
  obj->board = malloc(obj->rows * obj->cols);
  fread(obj->board, sizeof(char), obj->rows * obj->cols, f);
  return obj;
}

/* 게임 파일 저장기능 */
void tg_save(tetris_game *obj, FILE *f) {
  fwrite(obj, sizeof(tetris_game), 1, f);
  fwrite(obj->board, sizeof(char), obj->rows * obj->cols, f);
}

/* 단지 초기 디버깅을 위해 게임보드를 파일에 출력. */
void tg_print(tetris_game *obj, FILE *f) {
  int i, j;
  for (i = 0; i < obj->rows; i++) {
    for (j = 0; j < obj->cols; j++) {
      if (TC_IS_EMPTY(tg_get(obj, i, j))) {
        fputs(TC_EMPTY_STR, f);
      } else {
        fputs(TC_BLOCK_STR, f);
      }
    }
    fputc('\n', f);
  }
}
