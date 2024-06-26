#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"
#include "editor.h"

bool is_editor(editor* E) {
  if (E == NULL) return false;
  if (!is_gapbuf(E->buffer)) return false;
  if (E->row != gapbuf_row(E->buffer)) return false;
  if (E->col != gapbuf_col(E->buffer)) return false;
  if (E->rendercol != gapbuf_rendercol(E->buffer)) return false;
  if (E->numrows != gapbuf_numrows(E->buffer)) return false;
  return true;
}

editor* editor_new(void) {
  editor* E = xmalloc(sizeof(editor));

  E->buffer = gapbuf_new(10);
  E->row = 1;
  E->col = 0;
  E->rendercol = 0;
  E->numrows = 1;

  E->rowoff = 1; // first visible row is 1
  E->coloff = 0; // first visible col is 0

  E->filename = NULL;
  E->dirty = 0;
  E->quit_times = QUIT_TIMES;

  ENSURES(is_editor(E));
  return E;
}

/* editor operations */

void editor_forward(editor* E) {
  REQUIRES(is_editor(E));
  if (gapbuf_at_right(E->buffer)) return;

  ASSERT(!gapbuf_at_right(E->buffer));
  // character to the right of cursor
  char c = E->buffer->back[E->buffer->backlen - 1];
  gapbuf_forward(E->buffer);
  if (c == '\n') {
    E->row += 1;
    E->col = 0;
    E->rendercol = 0;
  }
  else if (c == '\t') {
    E->col += 1;
    E->rendercol += (TAB_STOP - 1) - (E->rendercol % TAB_STOP);
    E->rendercol += 1;
  }
  else {
    E->col += 1;
    E->rendercol += 1;
  }
  E->quit_times = QUIT_TIMES;
  ENSURES(is_editor(E));
}

void editor_backward(editor* E) {
  REQUIRES(is_editor(E));
  if (gapbuf_at_left(E->buffer)) return;

  ASSERT(!gapbuf_at_left(E->buffer));
  // character to the left of cursor
  char c = E->buffer->front[E->buffer->frontlen - 1];
  gapbuf_backward(E->buffer);
  if (c == '\n') {
    E->row -= 1;
    E->col = gapbuf_col(E->buffer);
    E->rendercol = gapbuf_rendercol(E->buffer);
  }
  else if (c == '\t') {
    E->col -= 1;
    E->rendercol = gapbuf_rendercol(E->buffer);
  }
  else {
    E->col -= 1;
    E->rendercol -= 1;
  }
  E->quit_times = QUIT_TIMES;
  ENSURES(is_editor(E));
}

void editor_up(editor* E) {
  REQUIRES(is_editor(E));
  
  // if already at first line, move to left most
  if (E->row == 1) {
    while (!gapbuf_at_left(E->buffer)) {
      editor_backward(E);
    }
    ENSURES(is_editor(E));
    return;
  }

  ASSERT(E->row > 1);
  size_t orig_row = E->row;
  size_t orig_col = E->col;
  while(E->row >= orig_row || E->col > orig_col) {
    editor_backward(E);
  }

  E->quit_times = QUIT_TIMES;
  ENSURES(is_editor(E));
}

void editor_down(editor* E) {
  REQUIRES(is_editor(E));

  // if already at final line, move to rightmost
  if (E->row == E->numrows) {
    while(!gapbuf_at_right(E->buffer)) {
      editor_forward(E);
    }
    ENSURES(is_editor(E));
    return;
  }

  ASSERT(E->row < E->numrows);
  size_t orig_row = E->row;
  size_t orig_col = E->col;
  while (E->row == orig_row) {
    editor_forward(E);
  }

  while (E->col < orig_col 
        && (!gapbuf_at_right(E->buffer)) 
        && E->buffer->back[E->buffer->backlen-1] != '\n') {
    editor_forward(E);
  }

  E->quit_times = QUIT_TIMES;
  ENSURES(is_editor(E));
}

void editor_endline(editor* E) {
  REQUIRES(is_editor(E));
  while (!gapbuf_at_right(E->buffer) 
          && E->buffer->back[E->buffer->backlen - 1] != '\n') {
    editor_forward(E);
  }
  E->quit_times = QUIT_TIMES;
  ENSURES(is_editor(E));
}

void editor_startline(editor* E) {
  REQUIRES(is_editor(E));
  while (!gapbuf_at_left(E->buffer)
          && E->buffer->front[E->buffer->frontlen - 1] != '\n') {
    editor_backward(E);
  }
  E->quit_times = QUIT_TIMES;
  ENSURES(is_editor(E));
}

void editor_insert(editor* E, char c) {
  REQUIRES(is_editor(E));
  gapbuf_insert(E->buffer, c);
  if (c == '\n') {
    E->row += 1;
    E->col = 0;
    E->rendercol = 0;
    E->numrows += 1;
  }
  else if (c == '\t') {
    E->col += 1;
    E->rendercol += (TAB_STOP - 1) - (E->rendercol % TAB_STOP);
    E->rendercol += 1;
  }
  else {
    E->col += 1;
    E->rendercol += 1;
  }
  E->dirty += 1;
  E->quit_times = QUIT_TIMES;
  ENSURES(is_editor(E));
}

void editor_delete(editor* E) {
  REQUIRES(is_editor(E));
  if (gapbuf_at_left(E->buffer)) return;

  ASSERT(!gapbuf_at_left(E->buffer));
  char c = gapbuf_delete(E->buffer);
  if (c == '\n') {
    E->row -= 1;
    E->col = gapbuf_col(E->buffer);
    E->rendercol = gapbuf_rendercol(E->buffer);
    E->numrows -= 1;
  }
  else if (c == '\t') {
    E->col -= 1;
    E->rendercol = gapbuf_rendercol(E->buffer);
  }
  else {
    E->col -= 1;
    E->rendercol -= 1;
  }
  E->dirty += 1;
  E->quit_times = QUIT_TIMES;
  ENSURES(is_editor(E));
}


/* free */

void editor_free(editor* E) {
  REQUIRES(is_editor(E));
  gapbuf_free(E->buffer);
  if (E->filename != NULL) free(E->filename);
  free(E);
}
