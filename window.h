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
#include <time.h>
#include <stdarg.h>
#include "gapbuf.h"
#include "editor.h"

#ifndef WINDOW_H
#define WINDOW_H

#define CTRL_KEY(k) ((k) & 0x1f)
#define MESSAGE_TIME (10)
#define QUIT_TIMES (3)

struct window_header {
  editor* editor;
  struct termios orig_terminal;
  size_t screenrows;                // total number of rows on screen
  size_t screencols;                // total number of cols on screen
  char message[80];
  time_t messageTime;
};
typedef struct window_header window;

void die(window* W, const char* s);               // debugging and display error

window* window_new(void);                         // create new window

void enableRawMode(window* W);                    // enable raw mode
void disableRawMode(window* W);                   // disable raw mode

int getCursorPosition(size_t* row, size_t* col);
int getScreenSize(size_t* numrows, size_t* numcols);
void getWindowSize(window* W);

void scroll(window* W);                           // adjust offset to scroll
void refresh(window* W);                          // redraw everything
void renderFileBar(window* W);                    // draw file bar
void renderText(window* W);                       // render text file
void renderStatusBar(window* W);                  // render status bar
void renderMessageBar(window* W);                 // render message bar
void setMessage(window* W, const char* fmt, ...); // set message bar message
void render(window* W);

int readKey(window* W);                           // read key presses
void moveCursor(window* W, int key);              // move cursor
void movePage(window* W, int key);                // next/previous page
void processKey(window* W, bool* go);             // process key press

void openFile(window* W, char* filename);         // open text file
void saveFile(window* W);                         // save edited file

void window_free(window* W);                      // free

#endif
