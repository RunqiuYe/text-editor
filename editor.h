#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gapbuf.h"

#ifndef EDITOR_H
#define EDITOR_H

struct editor_header {
  gapbuf* buffer;
  size_t row;
  size_t col;
  size_t numrows;
};
typedef struct editor_header editor;

bool is_editor(editor* E);              // representation invariant

editor* editor_new(); 	                // create a new and empty editor
void editor_forward(editor* E); 	      // move the cursor forward, to the right
void editor_backward(editor* E); 	      // move the cursor backward, to the left
void editor_insert(editor* E, char c); 	// insert c to the cursor’s left
void editor_delete(editor* E); 	        // remove the node to the cursor’s left

void editor_free(editor* E);            // free allocated space for editor

#endif
