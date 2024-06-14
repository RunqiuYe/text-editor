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
  editor* E;
  struct termios orig_terminal;
  size_t screenrows;                // total number of rows on screen
  size_t screencols;                // total number of cols on screen
};
typedef struct window_header window;

void die(window* W, const char* s);

window* window_new(void);

void enableRawMode(window* W);
void disableRawMode(window* W);
void getWindowSize(window* W);

void scroll(window* W);
void refresh(window* W);
void render(window* W);

int readKey(window* W);
void processKey(window* W, bool* go);

void openFile(window* W, char* filename);

void window_free(window* W);

#endif
