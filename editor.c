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

void die(editor* E, const char* s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  editor_free(E);
  perror(s);
  exit(1);
}

bool is_editor(editor* E) {
  if (E == NULL) return false;
  if (!is_gapbuf(E->buffer)) return false;
  if (E->row != gapbuf_row(E->buffer)) return false;
  if (E->col != gapbuf_col(E->buffer)) return false;
  if (E->numrows != gapbuf_numrows(E->buffer)) return false;
  return true;
}

editor* editor_new(void) {
  editor* E = xmalloc(sizeof(editor));

  E->buffer = gapbuf_new(10);
  E->row = 1;
  E->col = 0;
  E->numrows = 1;

  E->screenrows = 0;
  E->screencols = 0;
  E->rowoff = 0;
  E->coloff = 0;

  ENSURES(is_editor(E));
  return E;
}

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
  }
  else {
    E->col += 1;
  }
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
  }
  else {
    E->col -= 1;
  }
  ENSURES(is_editor(E));
}

void editor_insert(editor* E, char c) {
  REQUIRES(is_editor(E));
  gapbuf_insert(E->buffer, c);
  if (c == '\n') {
    E->row += 1;
    E->col = 0;
    E->numrows += 1;
  }
  else {
    E->col += 1;
  }
  ENSURES(is_editor(E));
}

void editor_delete(editor* E) {
  REQUIRES(is_editor(E));
  char c = gapbuf_delete(E->buffer);
  if (c == '\n') {
    E->row -= 1;
    E->col = gapbuf_col(E->buffer);
    E->numrows -= 1;
  }
  else {
    E->col -= 1;
  }
  ENSURES(is_editor(E));
}

void editor_open(editor* E, char* filename) {
  REQUIRES(is_editor(E) && filename != NULL);

  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    die(E, "fopen");
  }
  char c;
  while((c = fgetc(fp)) != EOF) {
    editor_insert(E, c);
  }
  fclose(fp);

  ENSURES(is_editor(E));
}

void editor_free(editor* E) {
  REQUIRES(is_editor(E));
  gapbuf_free(E->buffer);
  free(E);
}
