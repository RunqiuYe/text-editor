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
