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
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"
#include "editor.h"
#include "window.h"

#define MESSAGE_TIME (10)

/* TO-DO: renderText function, render tabs */
/* TO-DO: relevant tabs and cursor interaction */
/* TO-DO: prompt */

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

window* window_new(void) {
  window* W = xmalloc(sizeof(window));
  W->editor = editor_new();
  W->screenrows = 0;
  W->screencols = 0;
  if (tcgetattr(STDIN_FILENO, &W->orig_terminal) == -1) {
    die(W, "tcgetattr");
  }
  getWindowSize(W);

  W->message[0] = '\0';
  W->messageTime = 0;

  // status bar / ui offset
  W->screenrows -= 3;

  return W;
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

void scroll(window* W) {
  editor* E = W->editor;

  // rowoff = first visible row
  // coloff = first visible col
  // rowoff + screenrows = first invisible row
  // coloff + screencols = first invisible col
  if (E->row < E->rowoff) {
    E->rowoff = E->row;
  }
  if (E->row >= E->rowoff + W->screenrows) {
    E->rowoff = E->row - W->screenrows + 1;
  }
  if (E->col < E->coloff) {
    E->coloff = E->col;
  }
  if (E->col >= E->coloff + W->screencols) {
    E->coloff = E->col - W->screencols + 1;
  }
}

void refresh(window* W) {
  editor* E = W->editor;

  scroll(W);

  write(STDOUT_FILENO, "\x1b[?25l", 6);
  write(STDOUT_FILENO, "\x1b[H", 3);

  render(W);

  size_t cursorrow = E->row - E->rowoff + 2;
  size_t cursorcol = E->col - E->coloff + 1;

  // Move cursor to correct position
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%zu;%zuH", cursorrow, cursorcol);
  write(STDOUT_FILENO, buf, strlen(buf));

  write(STDOUT_FILENO, "\x1b[?25h", 6);
}

void renderFileBar(window* W) {
  editor* E = W->editor;

  // move cursor to first row
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%zu;1H", (size_t)1);
  write(STDOUT_FILENO, buf, strlen(buf));

  // first row to display file names
  char filenames[80];
  size_t len;
  len = snprintf(filenames, sizeof(filenames), "[ %.20s ]",
                E->filename != NULL ? E->filename: " [No Name] ");
  if (len > W->screencols) len = W->screencols;
  write(STDOUT_FILENO, "\x1b[7m", 4); // reverse color
  write(STDOUT_FILENO, filenames, len);
  write(STDOUT_FILENO, "\x1b[m", 3); // reset color
  write(STDOUT_FILENO, "\x1b[;100m", 7);
  while (len < W->screencols + 1) {
    write(STDOUT_FILENO, " ", 1);
    len += 1;
  }
  write(STDOUT_FILENO, "\x1b[m", 3); // reset color
  write(STDOUT_FILENO, "\x1b[K", 3);
  write(STDOUT_FILENO, "\n", 1);
}

void renderText(window* W) {
  /* TO-DO: rendering tabs */
  /* TO-DO: tabs and cursors */

  // move cursor to second row
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%zu;1H", (size_t)2);
  write(STDOUT_FILENO, buf, strlen(buf));

  editor* E = W->editor;
  gapbuf* gb = E->buffer;
  char* front = gb->front;
  char* back = gb->back;
  size_t frontlen = gb->frontlen;
  size_t backlen = gb->backlen;

  // rowoff = first visible row
  // coloff = first visible col
  // rowoff + screenrows = first invisible row
  // coloff + screencols = first invisible col

  // keep track of current row and col
  char c;
  size_t currow = 1;
  size_t curcol = 0;
  // render text in front of buffer
  for (size_t i = 0; i < frontlen; i++) {
    if (currow >= E->rowoff + W->screenrows) break;
    c = front[i];
    if (currow >= E->rowoff
      && currow < E->rowoff + W->screenrows
      && curcol >= E->coloff
      && curcol < E->coloff + W->screencols
    ) {
      if (c == '\n') write(STDOUT_FILENO, "\x1b[K", 3);
      write(STDOUT_FILENO, &c, 1);
    }
    if (c == '\n') {
      if (currow >= E->rowoff
        && currow < E->rowoff + W->screenrows
        && curcol < E->coloff
      ) {
        write(STDOUT_FILENO, "\x1b[K", 3);
        write(STDOUT_FILENO, "\n", 1);
      }
      currow += 1;
      curcol = 0;
    }
    else {
      curcol += 1;
    }
  }

  // render text in back of buffer
  for (size_t j = 0; j < backlen; j++) {
    if (currow >= E->rowoff + W->screenrows) break;
    c = back[backlen - j - 1];
    if (currow >= E->rowoff
      && currow < E->rowoff + W->screenrows
      && curcol >= E->coloff
      && curcol < E->coloff + W->screencols
    ) {
      if (c == '\n') write(STDOUT_FILENO, "\x1b[K", 3);
      write(STDOUT_FILENO, &c, 1);
    }
    if (c == '\n') {
      if (currow >= E->rowoff
        && currow < E->rowoff + W->screenrows
        && curcol < E->coloff
      ) {
        write(STDOUT_FILENO, "\x1b[K", 3);
        write(STDOUT_FILENO, "\n", 1);
      }
      currow += 1;
      curcol = 0;
    }
    else {
      curcol += 1;
    }
  }

  // clear line after render all text
  write(STDOUT_FILENO, "\x1b[K", 3);

  // if more rows empty after rendering, 
  // draw tilde on each empty row
  while (currow + 1 < E->rowoff + W->screenrows) {
    write(STDOUT_FILENO, "\x1b[K", 3);
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, "~", 1);
    currow += 1;
  }
}

void renderStatusBar(window* W) {
  editor* E = W->editor;

  // move cursor to second last row
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%zu;1H", W->screenrows + 2);
  write(STDOUT_FILENO, buf, strlen(buf));

  // second last row to display file status
  char status[80], rstatus[80];
  size_t len, rlen;
  len = snprintf(status, sizeof(status), 
                "%.20s - %zu lines %s",
                E->filename != NULL ? E->filename: "[No Name]",
                E->numrows,
                E->dirty != 0 ? "(modified)" : "");
  if (len > W->screencols) len = W->screencols;
  
  rlen = snprintf(rstatus, sizeof(rstatus),
                  "Position (%zu,%zu)", E->row, E->col);

  write(STDOUT_FILENO, "\x1b[7m", 4); // reverse color
  write(STDOUT_FILENO, status, len);
  for (size_t i = 0; i < W->screencols - len - rlen; i++) {
    write(STDOUT_FILENO, " ", 1);
  }
  write(STDOUT_FILENO, rstatus, rlen);
  write(STDOUT_FILENO, "\x1b[m", 3); // reset color
}

void renderMessageBar(window* W) {
  // move cursor to last row
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%zu;1H", W->screenrows + 3);
  write(STDOUT_FILENO, buf, strlen(buf));

  size_t msglen = strlen(W->message);
  if (msglen > W->screencols) msglen = W->screencols;
  if (msglen != 0 && time(NULL) - W->messageTime < MESSAGE_TIME) {
    write(STDOUT_FILENO, W->message, msglen);
  }
  write(STDOUT_FILENO, "\x1b[K", 3);
}

void setMessage(window* W, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(W->message, sizeof(W->message), fmt, ap);
  va_end(ap);
  W->messageTime = time(NULL);
}

void render(window* W) {
  renderFileBar(W);
  renderText(W);
  renderStatusBar(W);
  renderMessageBar(W);
}

int readKey(window* W) {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die(W, "read");
  }

  if (c == '\x1b') {
    // If start with <esc>, read more chars
    // for arrows keys and page up/down keys
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

void moveCursor(window* W, int key) {
  editor* E = W->editor;
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
      editor_up(E);
      break;
    }
    case ARROW_DOWN: {
      editor_down(E);
      break;
    }
  }
}

void processKey(window* W, bool* go) {
  editor* E = W->editor;
  int c = readKey(W);

  switch (c) {
    case CTRL_KEY('q'): {
      *go = false;
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      break;
    }
    case CTRL_KEY('s'): {
      saveFile(W);
      break;
    }
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT: {
      moveCursor(W, c);
      break;
    }
    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL_KEY: {
      if (c == DEL_KEY) moveCursor(W, ARROW_RIGHT);
      editor_delete(E);
      break;
    }
    case ENTER_KEY: {
      editor_insert(E, '\n');
      break;
    }
    default: {
      editor_insert(E, c);
      break;
    }
  }
}

void openFile(window* W, char* filename) {
  editor* E = W->editor;

  // get all text in file into buffer
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    die(W, "fopen");
  }
  
  E->filename = filename;

  char c;
  size_t count = 0;
  while((c = fgetc(fp)) != EOF) {
    count += 1;
    editor_insert(E, c);
  }
  fclose(fp);

  // move cursor to start of file
  for (size_t i = 0; i < count; i++) {
    editor_backward(E);
  }
  E->dirty = 0;
}

void saveFile(window* W) {
  editor* E = W->editor;
  if (E->filename == NULL) {
    return;
  }
  size_t len = E->buffer->backlen + E->buffer->frontlen;
  char* s = gapbuf_str(E->buffer);
  int fd = open(E->filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) {
      if (write(fd, s, len)) {
        close(fd);
        free(s);
        E->dirty = 0;
        setMessage(W, "%d bytes written to disk", len);
        return;
      }
    }
    close(fd);
  }
  free(s);
  setMessage(W, "Can't save! I/O error: %s", strerror(errno));
}

void window_free(window* W) {
  editor_free(W->editor);
  free(W);
}
