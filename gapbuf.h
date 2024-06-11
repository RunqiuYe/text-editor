#include <stdbool.h>
#include <stdlib.h>

#ifndef GAPBUF_H
#define GAPBUF_H

struct gapbuf_header {
  char* front;        // string before the gap
  char* back;         // string after the gap, inverted
  size_t frontlen;    // length of front string, frontlen < limit
  size_t backlen;     // length of back string, backlen < limit
  size_t limit;       // bytes allocated for front and back buffer
  					          // (we require them to have the same length),
  					          // limit > 0
};
typedef struct gapbuf_header gapbuf;

bool is_gapbuf(gapbuf* gb);                 // representation invariant

bool gapbuf_at_left(gapbuf* gb);            // return true if cursor(gap) is at leftmost position
bool gapbuf_at_right(gapbuf* gb);           // return true if cursor(gap) is at rightmost position

gapbuf* gapbuf_new(size_t init_limit);      // create new empty gap buffer
void gapbuf_forward(gapbuf* gb);            // move the cursor forward (to the right)
void gapbuf_backward(gapbuf* gb);           // move the cursor backward (to the left)
void gapbuf_insert(gapbuf* gb, char c);     // insert a character before cursor
char gapbuf_delete(gapbuf* gb);             // delete a character before cursor and return deleted char
char gapbuf_delete_right(gapbuf* gb);       // delete a character after the cursor and return deleted char

// void strbuf_add(strbuf *sb, char *str, size_t len);
// void strbuf_addstr(strbuf *sb, char *str);

size_t gapbuf_row(gapbuf* gb);                 // row of cursor position
size_t gapbuf_col(gapbuf* gb);                 // column of cursor position
size_t gapbuf_numrows(gapbuf* gb);             // number of rows in gap buffer

void gapbuf_free(gapbuf* gb);              // free allocated gapbuffer, and return the string contained
char* gapbuf_str(gapbuf* gb);               // the string contained in the text buffer
void gapbuf_print(gapbuf* gb);              // print the content of gapbuf for debugging

#endif
