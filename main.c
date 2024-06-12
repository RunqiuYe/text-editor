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

int main(void) {
  editor* E = editor_new();
  enableRawMode(E);

  bool go = true;

  while (go) {
    char c;
    read(STDIN_FILENO, &c, 1);
    if (iscntrl(c)) {
      printf("%d\n", c);
    } 
    else {
      printf("%d ('%c')\n", c, c);
    }
    if (c == CTRL_KEY('q')) go = false;
  }

  disableRawMode(E);
  editor_free(E);
  printf("Thanks for using RYe's editor");
  return 0;
}
