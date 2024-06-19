#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"
#include "editor.h"
#include "window.h"

int main(int argc, char* argv[]) {
  window* W = window_new();
  enableRawMode(W);
  if (argc >= 2) {
    openFile(W, argv[1]);
  }

  setMessage(W, "^Q — quit | ^S — save");

  bool* go = xcalloc(1, sizeof(bool));
  *go = true;

  while (*go) {
    refresh(W);
    processKey(W, go);
  }

  disableRawMode(W);
  window_free(W, argc);
  free(go);
  printf("Thanks for using RYe's editor\n");
  return 0;
}
