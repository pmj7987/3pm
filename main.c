#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ncurses.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "tetris.h"
#include "util.h"
#define COLS_PER_CELL 2 // 셀당 2열로 게임을 더욱 쾌적하게함
/* 특정 타입의 셀을 윈도에 인쇄하는 매크로. */
#define ADD_BLOCK(w,x) waddch((w),' '|A_REVERSE|COLOR_PAIR(x));     \
                       waddch((w),' '|A_REVERSE|COLOR_PAIR(x))
#define ADD_EMPTY(w) waddch((w), ' '); waddch((w), ' ')

/* ncurses 창에 테트리스보드를 출력. */
void display_board(WINDOW *w, tetris_game *obj) {
  int i, j;
  box(w, 0, 0);
  for (i = 0; i < obj->rows; i++) {
    wmove(w, 1 + i, 1);
    for (j = 0; j < obj->cols; j++) {
      if (TC_IS_FILLED(tg_get(obj, i, j))) {
        ADD_BLOCK(w,tg_get(obj, i, j));
      } else {
        ADD_EMPTY(w);
      }
    }
  }
  wnoutrefresh(w);
}

/* 디스플레이 화면에 테트리스 조각을 표시함. */
void display_piece(WINDOW *w, tetris_block block) {
  int b;
  tetris_location c;
  wclear(w);
  box(w, 0, 0);
  if (block.typ == -1) {
    wnoutrefresh(w);
    return;
  }
  for (b = 0; b < TETRIS; b++) {
    c = TETROMINOS[block.typ][block.ori][b];
    wmove(w, c.row + 1, c.col * COLS_PER_CELL + 1);
    ADD_BLOCK(w, TYPE_TO_CELL(block.typ));
  }
  wnoutrefresh(w);
}

/* 디스플레이 화면에 점수 정보 표시 */
void display_score(WINDOW *w, tetris_game *tg) {
  wclear(w);
  box(w, 0, 0);
  wprintw(w, "Score\n%d\n", tg->points);
  wprintw(w, "Level\n%d\n", tg->level);
  wprintw(w, "Lines\n%d\n", tg->lines_remaining);
  wnoutrefresh(w);
}

/* 아래 소스는 보스모드입니다. */
void boss_mode(void) {
  clear();
  printw("user@workstation-312:~/Documents/presentation $ ls -l\n"
         "total 528\n"
         "drwxr-xr-x 2 user users   4096 Jun  9 17:05 .\n"
         "drwxr-xr-x 4 user users   4096 Jun 10 09:52 ..\n"
         "-rw-r--r-- 1 user users  88583 Jun  9 14:13 figure1.png\n"
         "-rw-r--r-- 1 user users  65357 Jun  9 15:40 figure2.png\n"
         "-rw-r--r-- 1 user users   4469 Jun  9 16:17 presentation.aux\n"
         "-rw-r--r-- 1 user users  42858 Jun  9 16:17 presentation.log\n"
         "-rw-r--r-- 1 user users   2516 Jun  9 16:17 presentation.nav\n"
         "-rw-r--r-- 1 user users    183 Jun  9 16:17 presentation.out\n"
         "-rw-r--r-- 1 user users 349607 Jun  9 16:17 presentation.pdf\n"
         "-rw-r--r-- 1 user users      0 Jun  9 16:17 presentation.snm\n"
         "-rw-r--r-- 1 user users   9284 Jun  9 17:05 presentation.tex\n"
         "-rw-r--r-- 1 user users    229 Jun  9 16:17 presentation.toc\n"
         "\n"
         "user@workstation-312:~/Documents/presentation $ ");
  echo();
  timeout(-1);
  while (getch() != KEY_F(1));
  timeout(0);
  noecho();
  clear();
}

/* 게임을 저장하고 종료 */
void save(tetris_game *game, WINDOW *w) {
  FILE *f;

  wclear(w);
  box(w, 0, 0); // return the border
  wmove(w, 1, 1);
  wprintw(w, "Save and exit? [Y/n] ");
  wrefresh(w);
  timeout(-1);
  if (getch() == 'n') {
    timeout(0);
    return;
  }
  f = fopen("tetris.save", "w");
  tg_save(game, f);
  fclose(f);
  tg_delete(game);
  endwin();
  printf("Game saved to \"tetris.save\".\n");
  printf("Resume by passing the filename as an argument to this program.\n");
  exit(EXIT_SUCCESS);
}

/* COLOR 블록의 NCURSES 초기화 순서를 실행. */
void init_colors(void) {
  start_color();
  //init_color(COLOR_ORANGE, 1000, 647, 0);
  init_pair(TC_CELLI, COLOR_CYAN, COLOR_BLACK);
  init_pair(TC_CELLJ, COLOR_BLUE, COLOR_BLACK);
  init_pair(TC_CELLL, COLOR_WHITE, COLOR_BLACK);
  init_pair(TC_CELLO, COLOR_YELLOW, COLOR_BLACK);
  init_pair(TC_CELLS, COLOR_GREEN, COLOR_BLACK);
  init_pair(TC_CELLT, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(TC_CELLZ, COLOR_RED, COLOR_BLACK);
}

/* 테트리스 메인 게임 */
int main(int argc, char **argv) {
  tetris_game *tg;
  tetris_move move = TM_NONE;
  bool running = true;
  WINDOW *board, *next, *hold, *score;

  // 지정된 파일 이름을 불러옴
  if (argc >= 2) {
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
      perror("tetris");
      exit(EXIT_FAILURE);
    }
    tg = tg_load(f);
    fclose(f);
  } else {
    // 그렇지 않으면 새로운 게임을 시작
    tg = tg_create(22, 10);
  }

  // NCURSES 초기화
  initscr();
  cbreak();
  noecho();    
  keypad(stdscr, TRUE);
  timeout(0);
  curs_set(0);
  init_colors();

  // 인터페이스 섹션별로 새 창을 만듬.
  board = newwin(tg->rows + 2, 2 * tg->cols + 2, 0, 0);
  next  = newwin(6, 10, 0, 2 * (tg->cols + 1) + 1);
  hold  = newwin(6, 10, 7, 2 * (tg->cols + 1) + 1);
  score = newwin(6, 10, 14, 2 * (tg->cols + 1 ) + 1);

  // 게임 루프
  while (running) {
    running = tg_tick(tg, move);
    display_board(board, tg);
    display_piece(next, tg->next);
    display_piece(hold, tg->stored);
    display_score(score, tg);
    doupdate();
    sleep_milli(10);

    switch (getch()) {
    case KEY_LEFT:
      move = TM_LEFT;
      break;
    case KEY_RIGHT:
      move = TM_RIGHT;
      break;
    case KEY_UP:
      move = TM_CLOCK;
      break;
    case KEY_DOWN:
      move = TM_DROP;
      break;
    case 'q':
      running = false;
      move = TM_NONE;
      break;
    case 'p':
      wclear(board);
      box(board, 0, 0);
      wmove(board, tg->rows/2, (tg->cols*COLS_PER_CELL-6)/2);
      wprintw(board, "PAUSED");
      wrefresh(board);
      timeout(-1);
      getch();
      timeout(0);
      move = TM_NONE;
      break;
    case 'b':
      boss_mode();
      move = TM_NONE;
      break;
    case 's':
      save(tg, board);
      move = TM_NONE;
      break;
    case ' ':
      move = TM_HOLD;
      break;
    default:
      move = TM_NONE;
    }
  }

  // Ncurses 초기화 취소
  wclear(stdscr);
  endwin();

  // 종료 메시지 출력
  printf("Game over!\n");
  printf("You finished with %d points on level %d.\n", tg->points, tg->level);

  // 테트리스초기화 취소
  tg_delete(tg);
  return 0;
}
