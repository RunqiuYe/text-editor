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

/* TO-DO: modify save file and quit to support multiple files */

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
  (void) W;
  perror(s);
  exit(1);
}

window* window_new(void) {
  window* W = xmalloc(sizeof(window));
  W->editorList = xmalloc(2 * sizeof(editor*));
  W->editorList[0] = editor_new();
  W->editorList[1] = NULL;
  W->editorLim = 2;
  W->editorLen = 1;
  W->activeIndex = 0;
  W->editor = W->editorList[W->activeIndex];

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
  if (E->rendercol < E->coloff) {
    E->coloff = E->rendercol;
  }
  if (E->rendercol >= E->coloff + W->screencols) {
    E->coloff = E->rendercol - W->screencols + 1;
  }
}

void refresh(window* W) {
  editor* E = W->editor;

  scroll(W);

  write(STDOUT_FILENO, "\x1b[?25l", 6);
  write(STDOUT_FILENO, "\x1b[H", 3);

  render(W);

  size_t cursorrow = E->row - E->rowoff + 2;
  size_t cursorcol = E->rendercol - E->coloff + 1;

  // Move cursor to correct position
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%zu;%zuH", cursorrow, cursorcol);
  write(STDOUT_FILENO, buf, strlen(buf));

  write(STDOUT_FILENO, "\x1b[?25h", 6);
}

void renderFileBar(window* W) {
  // move cursor to first row
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%zu;1H", (size_t)1);
  write(STDOUT_FILENO, buf, strlen(buf));

  // first row to display file names
  char filenames[80];
  size_t len;
  size_t totalLen = 0;
  for (size_t i = 0; i < W->editorLen; i++) {
    editor* E = W->editorList[i];
    len = snprintf(filenames, sizeof(filenames), "[ %.20s ]",
                  E->filename != NULL ? E->filename: "[Untitled]");
    if (len > W->screencols - totalLen) len = W->screencols - totalLen;
    if (i == W->activeIndex) {
      write(STDOUT_FILENO, "\x1b[7m", 4); // reverse color
    }
    else {
      write(STDOUT_FILENO, "\x1b[;100m", 7); // grey background
    }
    write(STDOUT_FILENO, filenames, len);
    write(STDOUT_FILENO, "\x1b[m", 3); // reset color
    totalLen += len;
  }
  write(STDOUT_FILENO, "\x1b[;100m", 7);
  while (totalLen < W->screencols + 1) {
    write(STDOUT_FILENO, " ", 1);
    totalLen += 1;
  }
  write(STDOUT_FILENO, "\x1b[m", 3); // reset color
  // write(STDOUT_FILENO, "\x1b[K", 3);
  write(STDOUT_FILENO, "\n", 1);
}

void renderText(window* W) {
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
    // if go out of bound, stop immediately
    if (currow >= E->rowoff + W->screenrows) break;
    // current char to render
    c = front[i];

    // if current char is newline
    if (c == '\n') {
      if (currow >= E->rowoff && currow < E->rowoff + W->screenrows) {
        // don't clean line if cursor at end of terminal
        // otherwise last character is cleaned
        if (curcol < E->coloff + W->screencols - 1) {
          write(STDOUT_FILENO, "\x1b[K", 3);
        }
        write(STDOUT_FILENO, "\n", 1);
      }
      currow += 1;
      curcol = 0;
      continue;
    }

    // if current char is tab
    if (c == '\t') {
      if (currow >= E->rowoff && currow < E->rowoff + W->screenrows
        && curcol >= E->coloff && curcol < E->coloff + W->screencols
      ) {
        write(STDOUT_FILENO, " ", 1);
        curcol += 1;
        while (curcol % TAB_STOP != 0) {
          if (curcol < E->coloff + W->screencols) write(STDOUT_FILENO, " ", 1);
          curcol += 1;
        }
      }
      else {
        curcol += 1;
        while (curcol % TAB_STOP != 0) {
          curcol += 1;
        }
      }
      continue;
    }

    // default case
    if (currow >= E->rowoff && currow < E->rowoff + W->screenrows
      && curcol >= E->coloff && curcol < E->coloff + W->screencols
    ) {
      write(STDOUT_FILENO, &c, 1);
    }
    curcol += 1;
  }

  // render text in back of buffer
  for (size_t j = 0; j < backlen; j++) {
    // if go out of bound, stop immediately
    if (currow >= E->rowoff + W->screenrows) break;
    // current char to render
    c = back[backlen - j - 1];
    
    // if current char is newline
    if (c == '\n') {
      if (currow >= E->rowoff && currow < E->rowoff + W->screenrows) {
        // don't clean line if cursor at end of terminal
        // otherwise last character is cleaned
        if (curcol < E->coloff + W->screencols) {
          write(STDOUT_FILENO, "\x1b[K", 3);
        }
        write(STDOUT_FILENO, "\n", 1);
      }
      currow += 1;
      curcol = 0;
      continue;
    }

    // if current char is tab
    if (c == '\t') {
      if (currow >= E->rowoff && currow < E->rowoff + W->screenrows
        && curcol >= E->coloff && curcol < E->coloff + W->screencols
      ) {
        write(STDOUT_FILENO, " ", 1);
        curcol += 1;
        while (curcol % TAB_STOP != 0) {
          if (curcol < E->coloff + W->screencols) write(STDOUT_FILENO, " ", 1);
          curcol += 1;
        }
      }
      else {
        curcol += 1;
        while (curcol % TAB_STOP != 0) {
          curcol += 1;
        }
      }
      continue;
    }
    
    // default case
    if (currow >= E->rowoff && currow < E->rowoff + W->screenrows
      && curcol >= E->coloff && curcol < E->coloff + W->screencols
    ) {
      write(STDOUT_FILENO, &c, 1);
    }
    curcol += 1;
  }

  // clear line after render all text
  // don't clean line if cursor at end of terminal
  // otherwise last character is cleaned
  if (curcol < E->coloff + W->screencols - 1) {
    write(STDOUT_FILENO, "\x1b[K", 3);
  }

  // if more rows empty after rendering, 
  // draw tilde on each empty row
  while (currow + 1 < E->rowoff + W->screenrows) {
    write(STDOUT_FILENO, "\x1b[K", 3);
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, "~", 1);
    write(STDOUT_FILENO, "\x1b[K", 3);
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
                E->filename != NULL ? E->filename: "[Untitled]",
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
    case HOME_KEY:
    case CTRL_KEY('a'): {
      editor_startline(E);
      break;
    }
    case END_KEY:
    case CTRL_KEY('e'): {
      editor_endline(E);
      break;
    }
  }
}

void movePage(window* W, int key) {
  editor* E = W->editor;
  switch (key) {
  case PAGE_UP:
    case CTRL_KEY('w'): {
      while (E->row > E->rowoff) {
        editor_up(E);
      }
      // move up a page but 2 lines to keep continuity
      for (size_t i = 0; i < W->screenrows - 2; i++) {
        editor_up(E);
      }
      break;
    }

    case PAGE_DOWN:
    case CTRL_KEY('d'): {
      while (E->row < E->rowoff + W->screenrows - 1 && E->row < E->numrows) {
        editor_down(E);
      }
      // move down a page but 2 lines to keep continuity
      for (size_t i = 0; i < W->screenrows - 2; i++) {
        editor_down(E);
      }
      break;
    }
  }
}

void processKey(window* W, bool* go) {
  editor* E = W->editor;
  int c = readKey(W);

  switch (c) {
    case CTRL_KEY('o'): {
      char* filename = promptUser(W, "Open file: %s (Enter to confirm)", NULL);
      openFile(W, filename);
      break;
    }
    case CTRL_KEY('k'): {
      if (W->activeIndex == W->editorLen - 1) {
        W->activeIndex = 0;
      }
      else {
        W->activeIndex += 1;
      }
      W->editor = W->editorList[W->activeIndex];
      break;
    }

    case CTRL_KEY('j'): {
      if (W->activeIndex == 0) {
        W->activeIndex = W->editorLen - 1;
      }
      else {
        W->activeIndex -= 1;
      }
      W->editor = W->editorList[W->activeIndex];
      break;
    }

    case CTRL_KEY('x'): {
      closeFile(W, go);
      break;
    }

    case CTRL_KEY('q'): {
      while (W->editorLen > 0) {
        closeFile(W, go);
        if (W->editor->dirty != 0) break;
      }
      break;
    }

    case CTRL_KEY('s'): {
      saveFile(W);
      break;
    }

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT: 
    case HOME_KEY:
    case END_KEY:
    case CTRL_KEY('a'):
    case CTRL_KEY('e'): {
      moveCursor(W, c);
      break;
    }

    case PAGE_UP:
    case PAGE_DOWN: 
    case CTRL_KEY('w'): // simulate page up
    case CTRL_KEY('d'): // simulate page down
    {
      movePage(W, c);
      break;
    }

    case CTRL_KEY('f'): {
      find(W);
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

    case CTRL_KEY('l'):
    case '\x1b': {
      break;
    }
    
    default: {
      editor_insert(E, c);
      break;
    }
  }
}

char* promptUser(window* W, char* prompt, callback_fn* callback) {
  size_t bufsize = 128;
  size_t buflen = 0;
  char* buf = xmalloc(bufsize * sizeof(char));
  buf[0] = '\0';
  
  bool inPrompt = true;
  while (inPrompt) {
    setMessage(W, prompt, buf);
    refresh(W);
    int c = readKey(W);

    // backspace to delete
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
      if (buflen != 0) {
        buflen -= 1;
        buf[buflen] = '\0';
      }
    }

    // enter to confirm
    else if (c == ENTER_KEY) {
      inPrompt = false;

      if (buflen == 0) {
        setMessage(W, "");
        free(buf);
        return NULL;
      }

      else {
        setMessage(W, "");
        if (callback != NULL) (*callback)(W, buf, c);
        return buf;
      }
    }

    // esc to cancel
    else if (c == '\x1b') {
      setMessage(W, "");
      if (callback != NULL) (*callback)(W, buf, c);
      inPrompt = false;
      free(buf);
      return NULL;
    }

    // other keys to insert
    else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen] = c;
      buflen += 1;
      buf[buflen] = '\0';
    }

    if (callback != NULL) (*callback)(W, buf, c);
  }
  return buf;
}

void findCallback(window* W, char* query, int key) {
  static size_t lastMatch = 0; // store frontlen of lastMatch

  if (key == ENTER_KEY || key == '\x1b') {
    lastMatch = 0;
    return;
  }
  else if (key != ARROW_RIGHT && key != ARROW_DOWN) {
    lastMatch = 0;
  }

  if (query == NULL) return;

  editor* E = W->editor;
  char* s = gapbuf_str(E->buffer);
  char* match;
  match = strstr(s + lastMatch + strlen(query), query);
  if (match != NULL) {
    lastMatch = match - s;
  }
  else {
    match = strstr(s, query);
  }
  if (match != NULL) {
    size_t target_frontlen = match - s;
    while (E->buffer->frontlen > target_frontlen) editor_backward(E);
    while (E->buffer->frontlen < target_frontlen) editor_forward(E);
  }

  free(s);
}

void find(window* W) {
  editor* E = W->editor;
  size_t saved_frontlen = E->buffer->frontlen;

  char* query = promptUser(W, "Search: %s (Use Esc/Enter/Right)", findCallback);
  if (query != NULL) {
    free(query);
  }
  else {
    while (E->buffer->frontlen > saved_frontlen) editor_backward(E);
    while (E->buffer->frontlen < saved_frontlen) editor_forward(E);
  }
}

void openFile(window* W, char* filename) {
  editor* E = W->editor;

  // if current editor is not empty, open a new editor
  char* s = gapbuf_str(E->buffer);
  if (strcmp(s, "") != 0) {
    if (W->editorLen + 1 >= W->editorLim) {
      W->editorLim = 2 * W->editorLim;
      editor** newList = xcalloc(W->editorLim, sizeof(editor*));
      for (size_t i = 0; i < W->editorLen; i++) {
        newList[i] = W->editorList[i];
      }
      free(W->editorList);
      W->editorList = newList;
    }
    W->editorList[W->editorLen] = editor_new();
    W->editor = W->editorList[W->editorLen];
    W->activeIndex = W->editorLen;
    W->editorLen += 1;
  }
  free(s);

  E = W->editor;

  if (filename != NULL) {
    // get all text in file into buffer
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
      fp = fopen(filename, "w");
      fclose(fp);
      fp = fopen(filename, "r");
    }
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
  }

  E->dirty = 0;
}

void closeFile(window* W, bool* go) {
  editor* E = W->editor;
  if (E->dirty != 0 && E->quit_times > 0) {
    setMessage(W, "WARNING!!! File has unsaved changes. "
      "Press ^X %d more times to quit.", E->quit_times);
    E->quit_times -= 1;
    return;
  }
  setMessage(W, "");
  editor_free(W->editorList[W->activeIndex]);
  for (size_t i = W->activeIndex + 1; i < W->editorLen; i++) {
    W->editorList[i-1] = W->editorList[i];
  }
  W->editorList[W->editorLen - 1] = NULL;
  W->editorLen -= 1;
  if (W->editorLen == 0) {
    *go = false;
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    return;
  }
  if (W->activeIndex >= W->editorLen) {
    W->activeIndex -= 1;
  }
  W->editor = W->editorList[W->activeIndex];
}

void saveFile(window* W) {
  editor* E = W->editor;
  if (E->filename == NULL) {
    E->filename = promptUser(W, "Save as: %s", NULL);
    if (E->filename == NULL) {
      setMessage(W, "Save canceled");
      return;
    }
  }
  size_t len = E->buffer->backlen + E->buffer->frontlen;
  char* s = gapbuf_str(E->buffer);
  int fd = open(E->filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) {
      if ((size_t)write(fd, s, len) == len) {
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
  for (size_t i = 0; i < W->editorLen; i++) {
    editor_free(W->editorList[i]);
  }
  free(W->editorList);
  free(W);
}
