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

#ifndef WINDOW_H
#define WINDOW_H

#define CTRL_KEY(k) ((k) & 0x1f)

struct window_header {
  editor* editor;
  struct termios orig_terminal;
  size_t screenrows;                // total number of rows on screen
  size_t screencols;                // total number of cols on screen
};
typedef struct window_header window;

void die(window* W, const char* s);           // debugging and display error

window* window_new(void);                     // create new window

void enableRawMode(window* W);                // enable raw mode
void disableRawMode(window* W);               // disable raw mode
void refresh(window* W);                      // redraw everything
void processKey(window* W, bool* go);         // process key press
void openFile(window* W, char* filename);     // open text file
void window_free(window* W);                  // free

#endif
