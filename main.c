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
#include "terminal.h"

int main(int argc, char* argv[]) {
  editor* E = editor_new();
  enableRawMode(E);
  setWindowSize(E);
  if (argc >= 2) {
    editor_open(E, argv[1]);
  }

  bool* go = xcalloc(1, sizeof(bool));
  *go = true;

  while (*go) {
    refreshScreen(E);
    processKey(E, go);
  }

  disableRawMode(E);
  editor_free(E);
  free(go);
  printf("Thanks for using RYe's editor\n");
  return 0;
}
