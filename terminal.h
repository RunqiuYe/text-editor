#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include "gapbuf.h"
#include "editor.h"

#define CTRL_KEY(k) ((k) & 0x1f)

void die(const char* s);

void enableRawMode(editor* E);
void disableRawMode(editor* E);

int getWindowSize(size_t* numrows, size_t* numcols);
int getCursorPosition(size_t* row, size_t* col);
void setWindowSize(editor* E);

void refreshScreen(editor* E);
void render(editor* E);

void moveCursor(editor* E, int key);

int readKey(void);
void processKey(editor* E, bool* go);
