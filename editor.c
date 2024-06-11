#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"
#include "editor.h"

bool is_editor(editor* E) {
  if (E == NULL) return false;
  if (!is_gapbuf(E->buffer)) return false;
  if (E->row != gapbuf_row(E->buffer)) return false;
  if (E->col != gapbuf_col(E->buffer)) return false;
  if (E->numrows != gapbuf_numrows(E->buffer)) return false;
  return true;
}

editor* editor_new() {
  editor* E = xmalloc(sizeof(editor));
  E->buffer = gapbuf_new(10);
  E->row = 1;
  E->col = 0;
  E->numrows = 1;

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
