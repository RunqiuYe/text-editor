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
