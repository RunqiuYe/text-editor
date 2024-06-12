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
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"
#include "editor.h"

#define CTRL_KEY(k) ((k) & 0x1f)

enum key {
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

int getWindowSize(size_t* numrows, size_t* numcols) {
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

void setWindowSize(editor* E) {
  if (getWindowSize(&E->screenrows, &E->screencols) == -1) {
    die("getWindowSize");
  }
}

void render(editor* E) {
  for (size_t i = 0; i < E->screenrows; i++) {
    write(STDOUT_FILENO, "~", 1);
    if (i < E->screenrows - 1) {
      write(STDOUT_FILENO, "\n", 2);
    }
  }
}

void refreshScreen(editor* E) {
  write(STDOUT_FILENO, "\x1b[?25l", 6);
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  write(STDOUT_FILENO, "\x1b[?25h", 6);
  render(E);
}

void moveCursor(editor* E, int key) {
  switch (key) {
    case ARROW_LEFT: {
      editor_backward(E);
      break;
    }
    case ARROW_RIGHT: {
      editor_forward(E);
      break;
    }
    case ARROW_UP: {
      /*TO DO*/
      break;
    }
    case ARROW_DOWN: {
      /*TO DO*/
      break;
    }
  }
}

int readKey(void) {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  if (c == '\x1b') {
    // If start with <esc>, read more chars for arrows keys
    // and page up/down keys
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

    if (seq[0] == '[') {
      // Page Up and Page Down Key
      // Page Up <esc>[5~
      // Page Up <esc>[6~
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      }
      else {
        // Arrow Keys
        // Up <esc>[A
        // Down <esc>[B
        // Right <esc>[C
        // Left <esc>[D
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
      }
    }
    else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
    }
    return '\x1b';
  } 
  else {
    return c;
  }
}

void processKey(editor* E, bool* go) {
  int c = readKey();

  switch (c) {
    case CTRL_KEY('q'): {
      *go = false;
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      break;
    }
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT: {
      moveCursor(E, c);
    }
  }
}
