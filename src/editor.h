#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include "gapbuf.h"

#ifndef EDITOR_H
#define EDITOR_H

#define QUIT_TIMES (3)

struct editor_header {
  gapbuf* buffer;
  size_t row;           // current row
  size_t col;           // current col
  size_t rendercol;     // col when render, considering tabs
  size_t numrows;       // total number of rows
  size_t rowoff;        // first visible row
  size_t coloff;        // first visible col
  char* filename;       // name of file
  int dirty;            // dirty flag to show modified
  int quit_times;       // times need to quit
};
typedef struct editor_header editor;

bool is_editor(editor* E);                    // representation invariant

editor* editor_new(void); 	                  // create a new and empty editor

/* editor operations */

void editor_forward(editor* E);               // move the cursor forward, to the right
void editor_backward(editor* E);              // move the cursor backward, to the left
void editor_up(editor* E);
void editor_down(editor* E);
void editor_endline(editor* E);
void editor_startline(editor* E);
void editor_insert(editor* E, char c);        // insert c to the cursor’s left
void editor_delete(editor* E);                // remove the node to the cursor’s left

/* free */

void editor_free(editor* E);                  // free allocated space for editor

#endif
