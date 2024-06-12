#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"
#include "editor.h"

void die(const char* s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void enableRawMode(editor* E) {
  // store original terminal in editor
  if (tcgetattr(STDIN_FILENO, &E->orig_terminal) == -1) {
    die("tcgetattr");
  }

  // setup new editor in raw mode
  struct termios raw;
  if (tcgetattr(STDIN_FILENO, &raw) == -1) {
    die("tcgetattr");
  }
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

void disableRawMode(editor* E) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E->orig_terminal) == -1) {
    die("tcsetattr");
  }
}
