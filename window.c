#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "gapbuf.h"
#include "editor.h"
#include "window.h"

enum key {
  ENTER_KEY = 13,
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN,
};

void die(window* W, const char* s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  window_free(W);
  perror(s);
  exit(1);
}

void enableRawMode(window* W) {
  // store original terminal in editor
  if (tcgetattr(STDIN_FILENO, &W->orig_terminal) == -1) {
    die(W, "tcgetattr");
  }

  // setup new editor in raw mode
  struct termios raw;
  if (tcgetattr(STDIN_FILENO, &raw) == -1) {
    die(W, "tcgetattr");
  }
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die(W, "tcsetattr");
  }
}

void disableRawMode(window* W) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &W->orig_terminal) == -1) {
    die(W, "tcsetattr");
  }
}

int getCursorPosition(size_t* row, size_t* col) {
  char buf[32];
  size_t i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%zu;%zu", row, col) != 2) return -1;
  return 0;
}

int getScreenSize(size_t* numrows, size_t* numcols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return getCursorPosition(numrows, numcols);
  } 
  else {
    *numcols = ws.ws_col;
    *numrows = ws.ws_row;
    return 0;
  }
}

void getWindowSize(window* W) {
  if (getScreenSize(&W->screenrows, &W->screencols) == -1) {
    die(W, "getWindowSize");
  }
}

