#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"

bool is_gapbuf(gapbuf* gb) {
  if (gb == NULL) return false;
  if (gb->front == NULL) return false;
  if (gb->back == NULL) return false;
  if (gb->limit <= 0) return false;
  if (gb->frontlen >= gb->limit) return false;
  if (gb->backlen >= gb->limit) return false;
  if (gb->front[gb->frontlen] != '\0') return false;
  if (gb->back[gb->backlen] != '\0') return false;
  if (strlen(gb->front) != gb->frontlen) return false;
  if (strlen(gb->back) != gb->backlen) return false;
  // \length(gb->front) = \length(gb->back) = fb->limit
  return true;
}

bool gapbuf_at_left(gapbuf *gb) {
  REQUIRES(is_gapbuf(gb));
  return gb->frontlen == 0;
}

bool gapbuf_at_right(gapbuf *gb) {
  REQUIRES(is_gapbuf(gb));
  return gb->backlen == 0;
}

gapbuf* gapbuf_new(size_t init_limit) {
  REQUIRES(init_limit > 0);
  gapbuf* gb = xmalloc(sizeof(gapbuf));
  gb->front = xcalloc(init_limit, sizeof(char));
  gb->back = xcalloc(init_limit, sizeof(char));
  gb->frontlen = 0;
  gb->backlen = 0;
  gb->limit = init_limit;

  ENSURES(is_gapbuf(gb));
  return gb;
}

void gapbuf_forward(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  REQUIRES(!gapbuf_at_right(gb));

  if (gb->frontlen + 2 >= gb->limit) {
    // if front buffer is full, reallocate and double size
    size_t new_limit = 3 * gb->limit;
    char* new_front = xcalloc(new_limit, sizeof(char));
    char* new_back = xcalloc(new_limit, sizeof(char));
    
    size_t i;
    for (i = 0; i < gb->frontlen; i++) {
      new_front[i] = gb->front[i];
    }
    for (i = 0; i < gb->backlen; i++) {
      new_back[i] = gb->back[i];
    }
    free(gb->front);
    free(gb->back);
    gb->limit = new_limit;
    gb->front = new_front;
    gb->back = new_back;
  }
  ASSERT(gb->frontlen + 2 < gb->limit);

  char c = gb->back[gb->backlen - 1];
  ASSERT(c != '\0');
  gb->front[gb->frontlen] = c;
  gb->front[gb->frontlen + 1] = '\0';
  gb->back[gb->backlen - 1]  = '\0';
  gb->frontlen += 1;
  gb->backlen -= 1;

  ENSURES(is_gapbuf(gb));
}

void gapbuf_backward(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  REQUIRES(!gapbuf_at_left(gb));

  if (gb->backlen + 2 >= gb->limit) {
    // if backbuffer is full, reallocate and double size
    size_t new_limit = 3 * gb->limit;
    char* new_front = xcalloc(new_limit, sizeof(char));
    char* new_back = xcalloc(new_limit, sizeof(char));
    
    size_t i;
    for (i = 0; i < gb->frontlen; i++) {
      new_front[i] = gb->front[i];
    }
    for (i = 0; i < gb->backlen; i++) {
      new_back[i] = gb->back[i];
    }
    free(gb->front);
    free(gb->back);
    gb->limit = new_limit;
    gb->front = new_front;
    gb->back = new_back;
  }
  ASSERT(gb->backlen + 2 < gb->limit);

  char c = gb->front[gb->frontlen - 1];
  ASSERT(c != '\0');
  gb->back[gb->backlen] = c;
  gb->back[gb->backlen + 1] = '\0';
  gb->front[gb->frontlen - 1] = '\0';
  gb->frontlen -= 1;
  gb->backlen += 1;

  ENSURES(is_gapbuf(gb));
}

void gapbuf_insert(gapbuf* gb, char c) {
  REQUIRES(is_gapbuf(gb));

  if (gb->frontlen + 2 >= gb->limit) {
    // if front buffer is full, reallocate and double size
    size_t new_limit = 3 * gb->limit;
    char* new_front = xcalloc(new_limit, sizeof(char));
    char* new_back = xcalloc(new_limit, sizeof(char));
    
    size_t i;
    for (i = 0; i < gb->frontlen; i++) {
      new_front[i] = gb->front[i];
    }
    for (i = 0; i < gb->backlen; i++) {
      new_back[i] = gb->back[i];
    }
    free(gb->front);
    free(gb->back);
    gb->limit = new_limit;
    gb->front = new_front;
    gb->back = new_back;
  }
  ASSERT(gb->frontlen + 2 < gb->limit);

  gb->front[gb->frontlen] = c;
  gb->frontlen += 1;

  ENSURES(is_gapbuf(gb));
}

char gapbuf_delete(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  REQUIRES(!gapbuf_at_left(gb));

  char c = gb->front[gb->frontlen - 1];
  gb->front[gb->frontlen - 1] = '\0';
  gb->frontlen -= 1;

  ENSURES(is_gapbuf(gb));
  return c;
}

char gapbuf_delete_right(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  REQUIRES(!gapbuf_at_right(gb));

  gapbuf_forward(gb);
  char c = gapbuf_delete(gb);

  ENSURES(is_gapbuf(gb));
  return c;
}


size_t gapbuf_row(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  size_t i;
  int row = 1;
  for (i = 0; i < gb->frontlen; i++) {
    if (gb->front[i] == '\n') row++;
  }
  return row;
}

size_t gapbuf_col(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  size_t i;
  int col = 0;
  for (i = 0; i < gb->frontlen; i++) {
    if (gb->front[i] == '\n') col = 0;
    else col++;
  }
  return col;
}

size_t gapbuf_rendercol(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  int rendercol = 0;
  for (size_t i = 0; i < gb->frontlen; i++) {
    if (gb->front[i] == '\n') rendercol = 0;
    else if (gb->front[i] == '\t') {
      rendercol += (TAB_STOP - 1) - (rendercol % TAB_STOP);
      rendercol += 1;
    }
    else rendercol += 1;
  }
  return rendercol;
}

size_t gapbuf_numrows(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  size_t row = 1;
  for (size_t i = 0; i < gb->frontlen; i++) {
    if (gb->front[i] == '\n') row += 1;
  }
  for (size_t j = 0; j < gb->backlen; j++) {
    if (gb->back[j] == '\n') row += 1;
  }
  return row;
}

void gapbuf_free(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  free(gb->front);
  free(gb->back);
  free(gb);
}

char* gapbuf_str(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  int len = gb->frontlen + gb->backlen;
  char* s = xcalloc(len+1, sizeof(char));
  size_t i;
  for (i = 0; i < gb->frontlen; i++) {
    s[i] = gb->front[i];
  }
  size_t j;
  for (j = 0; j < gb->backlen; j++) {
    s[len-1-j] = gb->back[j];
  }
  return s;
}

void gapbuf_print(gapbuf* gb) {
  REQUIRES(is_gapbuf(gb));
  int len = gb->frontlen + gb->backlen;
  char* s = xcalloc(len+3, sizeof(char));
  size_t i;
  for (i = 0; i < gb->frontlen; i++) {
    s[i] = gb->front[i];
  }
  ASSERT(i == gb->frontlen);
  s[i] = '[';
  s[i+1] = ']';
  size_t j;
  for (j = 0; j < gb->backlen; j++) {
    s[len+1-j] = gb->back[j];
  }
  printf("-- gapbuf print -- \n");
  printf("content: %s\n", s);
  printf("limit: %zu\n", gb->limit);
  printf("------------------ \n");
  free(s);
}
