#ifndef TETRIS_H
#define TETRIS_H

#include <stdio.h> // for FILE
#include <stdbool.h> // for bool

/* 테트리스 블럭을 대응하는 셀로 변환합니다. */
#define TYPE_TO_CELL(x) ((x)+1)

/* 테트리스보드의 인쇄방법을 나타내는 문자열. */
#define TC_EMPTY_STR " "
#define TC_BLOCK_STR "\u2588"
#define TC_IS_EMPTY(x) ((x) == TC_EMPTY)
#define TC_IS_FILLED(x) (!TC_IS_EMPTY(x))
#define TETRIS 4 // 테트리스 셀 갯수(셀은 테트리스 보드의 1x1 블록을 의미함.)
#define NUM_TETROMINOS 7 // 테트리스 모양 블록수
#define NUM_ORIENTATIONS 4 // 테트리스 블록 방향 갯수
/* 레벨상수 */
#define MAX_LEVEL 19
#define LINES_PER_LEVEL 10

typedef enum {
  TC_EMPTY, TC_CELLI, TC_CELLJ, TC_CELLL, TC_CELLO, TC_CELLS, TC_CELLT, TC_CELLZ
} tetris_cell;

/* "type"는 테트리스 블럭의 형태/모양이다. 방향은 포함하지 않는다. */
typedef enum {
  TET_I, TET_J, TET_L, TET_O, TET_S, TET_T, TET_Z
} tetris_type;

typedef struct {
  int row;
  int col;
} tetris_location;

/* "block"은 테트로미노에 관한 정보를 격납하는 구조. 구체적으로는 어떠한 타입인지, 어떠한 방향인지, 어떠한 장소인지를 나타낸다. */
typedef struct {
  int typ;
  int ori;
  tetris_location loc;
} tetris_block;

/* 게임에 입력되는 가능한 동작 기능 */
typedef enum {
  TM_LEFT, TM_RIGHT, TM_CLOCK, TM_COUNTER, TM_DROP, TM_HOLD, TM_NONE
} tetris_move;

typedef struct {

  int rows;
  int cols;
  char *board;
  /* 점수 매기기 */
  int points;
  int level;
  /* 현재 다운된 블록, 다음 블록은 이 블록 뒤에 있는 블록이다. 스왑 아웃이 가능한 블록을 저장하는 기능 */
  tetris_block falling;
  tetris_block next;
  tetris_block stored;
  /* 블록이 내려갈 때까지 게임이 구동되는 횟수. */
  int ticks_till_gravity;
  /* 다음 단계로 나아갈 때까지의 줄수 */
  int lines_remaining;
} tetris_game;


extern tetris_location TETROMINOS[NUM_TETROMINOS][NUM_ORIENTATIONS][TETRIS];

extern int GRAVITY_LEVEL[MAX_LEVEL+1];

// data 구조조작
void tg_init(tetris_game *obj, int rows, int cols);
tetris_game *tg_create(int rows, int cols);
void tg_destroy(tetris_game *obj);
void tg_delete(tetris_game *obj);
tetris_game *tg_load(FILE *f);
void tg_save(tetris_game *obj, FILE *f);

// 메모리에 관련되지 않는 퍼블릭 방식
char tg_get(tetris_game *obj, int row, int col);
bool tg_check(tetris_game *obj, int row, int col);
bool tg_tick(tetris_game *obj, tetris_move move);
void tg_print(tetris_game *obj, FILE *f);

#endif // TETRIS_H
