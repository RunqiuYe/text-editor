#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"

bool is_gapbuf(gapbuf* gb) {
  if (gb == NULL) return false;
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

  if (gb->frontlen + 1 >= gb->limit) {
    // if front buffer is full, reallocate and double size
    size_t new_limit = 2 * gb->limit;
    char* new_front = xcalloc(new_limit, sizeof(char));
    char* new_back = xcalloc(new_limit, sizeof(char));
    
    int i;
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
  ASSERT(gb->frontlen + 1 < gb->limit);

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

  if (gb->backlen + 1 >= gb->limit) {
    // if backbuffer is full, reallocate and double size
    size_t new_limit = 2 * gb->limit;
    char* new_front = xcalloc(new_limit, sizeof(char));
    char* new_back = xcalloc(new_limit, sizeof(char));
    
    int i;
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
  ASSERT(gb->backlen + 1 < gb->limit);

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

  if (gb->frontlen + 1 >= gb->limit) {
    // if front buffer is full, reallocate and double size
    size_t new_limit = 2 * gb->limit;
    char* new_front = xcalloc(new_limit, sizeof(char));
    char* new_back = xcalloc(new_limit, sizeof(char));
    
    int i;
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
  ASSERT(gb->frontlen + 1 < gb->limit);

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


