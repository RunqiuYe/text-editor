#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"

bool is_gapbuf(gapbuf* gb) {
    if (gb == NULL) return false;
    if (gb->frontlim <= 0 || gb->backlim <= 0) return false;
    if (gb->frontlen >= gb->frontlim) return false;
    if (gb->backlen >= gb->backlim) return false;
    if (strlen(gb->front) != gb->frontlen) return false;
    if (strlen(gb->back) != gb->backlen) return false;
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

gapbuf* gapbuf_new() {
    gapbuf* gb = xmalloc(sizeof(gapbuf));
    gb->front = xcalloc(2, sizeof(char));
    gb->back = xcalloc(2, sizeof(char));
    gb->frontlen = 0;
    gb->backlen = 0;
    gb->frontlim = 1;
    gb->backlim = 1;

    ENSURES(is_gapbuf(gb));
    return gb;
}
