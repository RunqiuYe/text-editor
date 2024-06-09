## `lovas-E0.c0`

```c
/**************************************************************************/
/*              COPYRIGHT Carnegie Mellon University 2023                 */
/* Do not post this file or any derivative on a public site or repository */
/**************************************************************************/
/***************************************
 ** E0: a hacky text editor front-end **
 **                                   **
 **         by William Lovas          **
 **         and Robert Simmons        **
 **          using C0/curses          **
 **                                   **
 ***************************************/

 /******************************************************************************
           Once you implement required functionality, run
           cc0 -d -x -w tbuf.c0 editor.c0 lovas-E0.c0
 ******************************************************************************/

#use <conio>
#use <curses>
#use <string>


void render_topbar(WINDOW* W)
//@requires W != NULL;
{
  c_werase(W);
  for (int i = cc_getbegx(W); i < cc_getmaxx(W); i++)
    c_waddch(W, cc_highlight(char_ord(' ')));

  c_wmove(W, 0, 1);
  cc_wreverseon(W);
  c_waddstr(W, "E0, the minimalist editor -- ^X to exit, ^L to refresh");
  cc_wreverseoff(W);
}

void render_botbar(WINDOW* W, editor* E)
//@requires W != NULL;
//@requires is_editor(E);
{
  c_werase(W);
  for (int i = cc_getbegx(W); i < cc_getmaxx(W); i++)
    c_waddch(W, cc_highlight(char_ord(' ')));

  c_wmove(W, 0, 1);
  cc_wreverseon(W);

  string msg = "Position (";
  msg = string_join(msg, string_fromint(E->row));
  msg = string_join(msg, ",");
  msg = string_join(msg, string_fromint(E->col));
  msg = string_join(msg, ")");
  c_waddstr(W, msg);
  cc_wreverseoff(W);
}

void render_buf(WINDOW* W, tbuf* B)
//@requires is_tbuf(B);
//@ensures is_tbuf(B);
{
  c_wmove(W, 0, 0);
  c_werase(W);

  for (dll* p = B->start->next; p != B->end; p = p->next) {
    char c = p->data;

    // The display character is either ' ' (if it's a newline) or the char
    // The display character is what gets highlighted if we're at the point
    int display = c == '\n' ? char_ord(' ') : char_ord(c);
    if (p == B->cursor) display = cc_highlight(display);

    if (cc_gety(W) == cc_getmaxy(W) - 1) {
      // Special corner cases: last line of the buffer
      if (c != '\n' && cc_getx(W) < cc_getmaxx(W) - 1) {
        c_waddch(W, display); // Show a regular character (common case)
      } else {
        if (c == '\n') c_waddch(W, display);
        while (cc_getx(W) != cc_getmaxx(W) - 1) c_waddch(W, char_ord(' '));
        c_waddch(W, char_ord('>'));
        return;
      }

    } else {
      // Normal cases: in the buffer
      if (c != '\n' && cc_getx(W) < cc_getmaxx(W) - 1) {
        c_waddch(W, display); // Show a regular character (common case)
      } else if (c == '\n' && cc_getx(W) < cc_getmaxx(W) - 1) {
        c_waddch(W, display); // Newline (common case)
        c_waddch(W, char_ord('\n'));
      } else if (c == '\n') {
        c_waddch(W, display); // Newline (edge case, newline at end of line)
      } else {
        c_waddch(W, char_ord('\\'));
        c_waddch(W, display); // Wrap to the next line
      }
    }
  }

  // We're at the end of the buffer. This only matters if end = cursor
  if (B->cursor == B->end) {
    c_waddch(W, cc_highlight(char_ord(' ')));
  }
}

/** main: look the other way if you've ever programmed using curses **/

int main() {
  WINDOW* mainwin = c_initscr();
  c_cbreak();
  c_noecho();
  c_keypad(mainwin, true);
  int vis = c_curs_set(0);

  int ncols = cc_getmaxx(mainwin);
  int nlines = cc_getmaxy(mainwin);
  int begx = cc_getbegx(mainwin);
  int begy = cc_getbegy(mainwin);

  WINDOW* canvas = c_subwin(mainwin,
  //                        nlines - 3  /* lines: save 3 for status info */,
                            nlines - 2  /* lines: save 2 for status info */,
                            ncols       /* cols: same as main window */,
                            begy + 1    /* begy: skip top status bar */,
                            begx        /* begx: same as main window */);

  WINDOW* topbar = c_subwin(mainwin, 1 /* lines */, ncols, begy, begx);
  WINDOW* botbar = c_subwin(mainwin, 1 /* lines */, ncols, nlines - 2, begx);
  //WINDOW* minibuf = c_subwin(mainwin, 1 /* lines */, ncols, nlines - 1, begx);

  editor* E = editor_new();
  render_topbar(topbar);
  render_botbar(botbar, E);

  bool go = true;
  while (go) {
    render_buf(canvas, E->buffer);
    c_wrefresh(canvas);
    //vrender_buf(B, minibuf);
    render_botbar(botbar, E);
    c_wrefresh(botbar);

    int c = c_getch();
    if (c == 24) /* ^X */ { go = false; }
    else if (c == 12) /* ^L */ {
      c_wclear(mainwin);
      render_topbar(topbar);
      c_wrefresh(mainwin);
    }
    else if (cc_key_is_backspace(c)) { editor_delete(E); }
    else if (cc_key_is_left(c))  { editor_backward(E); }
    else if (cc_key_is_right(c)) { editor_forward(E); }
    else if (cc_key_is_up(c))    { editor_up(E); }
    else if (cc_key_is_down(c))  { editor_down(E); }
    else if (cc_key_is_enter(c)) { editor_insert(E, '\n'); }
    else if (0 <= c && c <= 127) { editor_insert(E, char_chr(c)); }
  }

  c_curs_set(vis);
  c_endwin();

  printf("thanks for flying E !\b\b"); /* <- wing commander homage -wjl */

  return 0;
}

```



## `c0lib/curses-view.h0`

```c
/* curses functions c_*, my functions cc_* -- for details, "man ncurses" */

struct _win_st;
typedef struct _win_st WINDOW;

WINDOW *c_initscr();
void c_cbreak();
void c_noecho();
void c_keypad(WINDOW *w, bool b);
int c_getch();
void c_addch(int c);
void c_waddch(WINDOW *w, int c);
void c_waddstr(WINDOW *w, string s);
void c_wstandout(WINDOW *w);
void c_wstandend(WINDOW *w);
void c_delch();
void c_erase();
void c_werase(WINDOW *w);
void c_wclear(WINDOW *w);
void c_move(int y, int x);
void c_wmove(WINDOW *w, int y, int x);
void c_refresh();
void c_wrefresh(WINDOW *w);
int c_endwin();
int c_curs_set(int visibility);
WINDOW *c_subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);

void cc_wboldon(WINDOW *w);
void cc_wboldoff(WINDOW *w);
void cc_wdimon(WINDOW *w);
void cc_wdimoff(WINDOW *w);
void cc_wreverseon(WINDOW *w);
void cc_wreverseoff(WINDOW *w);
void cc_wunderon (WINDOW *w);
void cc_wunderoff(WINDOW *w);

int cc_highlight(int c);
int cc_getx(WINDOW *w);
int cc_gety(WINDOW *w);
int cc_getmaxx(WINDOW *w);
int cc_getmaxy(WINDOW *w);
int cc_getbegx(WINDOW *w);
int cc_getbegy(WINDOW *w);

bool cc_key_is_enter(int key);
bool cc_key_is_backspace(int key);
bool cc_key_is_left(int key);
bool cc_key_is_right(int key);
bool cc_key_is_up(int key);
bool cc_key_is_down(int key);
```



## `c0lib/curses.h0`

```c
/* curses functions c_*, my functions cc_* -- for details, "man ncurses" */

struct _win_st;
typedef struct _win_st WINDOW;

WINDOW *c_initscr();
void c_cbreak();
void c_noecho();
void c_keypad(WINDOW *w, bool b);
int c_getch();
void c_addch(int c);
void c_waddch(WINDOW *w, int c);
void c_waddstr(WINDOW *w, string s);
void c_wstandout(WINDOW *w);
void c_wstandend(WINDOW *w);
void c_delch();
void c_erase();
void c_werase(WINDOW *w);
void c_wclear(WINDOW *w);
void c_move(int y, int x);
void c_wmove(WINDOW *w, int y, int x);
void c_refresh();
void c_wrefresh(WINDOW *w);
int c_endwin();
int c_curs_set(int visibility);
WINDOW *c_subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);

void cc_wboldon(WINDOW *w);
void cc_wboldoff(WINDOW *w);
void cc_wdimon(WINDOW *w);
void cc_wdimoff(WINDOW *w);
void cc_wreverseon(WINDOW *w);
void cc_wreverseoff(WINDOW *w);
void cc_wunderon (WINDOW *w);
void cc_wunderoff(WINDOW *w);

int cc_highlight(int c);
int cc_getx(WINDOW *w);
int cc_gety(WINDOW *w);
int cc_getmaxx(WINDOW *w);
int cc_getmaxy(WINDOW *w);
int cc_getbegx(WINDOW *w);
int cc_getbegy(WINDOW *w);

bool cc_key_is_enter(int key);
bool cc_key_is_backspace(int key);
bool cc_key_is_left(int key);
bool cc_key_is_right(int key);
bool cc_key_is_up(int key);
bool cc_key_is_down(int key);
```



## `tbuf.c0`

```c
/**************************************************************************/
/*              COPYRIGHT Carnegie Mellon University 2023                 */
/* Do not post this file or any derivative on a public site or repository */
/**************************************************************************/
/*
 * Text buffers as doubly linked lists.
 *
 * 15-122 Principles of Imperative Computation */

#use <conio>
#use <util>
#use <string>

/*** Interface ***/

typedef struct dll_node dll;
struct dll_node {
  dll* next;
  char data;
  dll* prev;
};

typedef struct tbuf_header tbuf;
struct tbuf_header {
  dll* start;  /* the first node of the list (on the left)  */
  dll* cursor; /* the position where the cursor is displayed */
  dll* end;    /* the last node of the list (on the right) */
};

bool is_tbuf(tbuf* B);

bool tbuf_at_left(tbuf* B);            /* Returns true if the cursor       */
                                       /* is in leftmost position          */
bool tbuf_at_right(tbuf* B);           /* Returns true if the cursor       */
                                       /* is in rightmost position         */

tbuf* tbuf_new();                      /* Creates a new, empty text buffer */
void tbuf_forward(tbuf* B);            /* Moves the cursor forward (right) */
void tbuf_backward(tbuf* B);           /* Moves the cursor backward (left) */
void tbuf_insert(tbuf* B, char c);     /* Insert a character before cursor */
char tbuf_delete(tbuf* B);             /* Remove character before cursor   */
                                       /* (and returns the deleted char)   */

int tbuf_row(tbuf* B);                 /* Returns the row of the cursor    */
int tbuf_col(tbuf* B);                 /* Returns the column of the cursor */

/*** Implementation ***/
int count_tbuf(tbuf* B) 
// Count the length of the segment to help check mirror
// Also check if next and prev sequence meet cursor
// If not a valid segment, \result = -1
{
  if (B == NULL) return -1;
  int count_f = 0;
  int count_b = 0;
  bool meet_cur_f = false;
  bool meet_cur_b = false;
  dll* p_f = B->start->next;
  dll* p_b = B->end->prev;

  while (p_f != NULL && p_f != B->end)
  {
    if (p_f == B->cursor) {meet_cur_f = true;}
    p_f = p_f->next;
    count_f++;
  }
  while (p_b != NULL && p_b != B->start)
  {
    if (p_b == B->cursor) {meet_cur_b = true;}
    p_b = p_b->prev;
    count_b++;
  }

  bool meet_cur = (meet_cur_f && meet_cur_b) || (B->cursor == B->end);
  if (p_b == B->start && p_f == B->end
      && meet_cur && count_f == count_b) return count_f;
  return -1;
}

bool is_tbuf(tbuf* B) {
  if (B == NULL) return false;
  if (B->start == NULL || B->end == NULL || B->cursor == NULL) {
    return false;
  }
  int tbuf_len = count_tbuf(B);
  if (tbuf_len == -1) return false;

  // Now check the mirror of next and prev
  dll* p_f = B->start->next;
  dll* cur_f = p_f;
  for (int i = 0; i < tbuf_len; i++)
  {
    dll* cur_b = B->end;
    for (int j = 0; j < tbuf_len - i; j++) {
      cur_b = cur_b->prev;
    }
    if (cur_b != cur_f) return false;
    cur_f = cur_f->next;
  }
  return true;
}

bool tbuf_at_left(tbuf* B)
//@requires is_tbuf(B);
//@ensures is_tbuf(B);
{
  return B->cursor == B->start->next;
}

bool tbuf_at_right(tbuf* B)
//@requires is_tbuf(B);
//@ensures is_tbuf(B);
{
  return B->cursor == B->end;
}

tbuf* tbuf_new()
//@ensures is_tbuf(\result);
{
  tbuf* B = alloc(tbuf);
  dll* d0 = alloc(dll);
  dll* d1 = alloc(dll);
  d1->prev = d0;
  d0->next = d1;
  B->start = d0;
  B->cursor = d1;
  B->end = d1;
  return B;
}

void tbuf_forward(tbuf* B)
//@requires is_tbuf(B) && !tbuf_at_right(B);
//@ensures is_tbuf(B);
{
  B->cursor = B->cursor->next;
}

void tbuf_backward(tbuf* B)
//@requires is_tbuf(B) && !tbuf_at_left(B);
//@ensures is_tbuf(B);
{
  B->cursor = B->cursor->prev;
}

void tbuf_insert(tbuf* B, char c)
//@requires is_tbuf(B);
//@ensures is_tbuf(B) && !tbuf_at_left(B);
{
  dll* d = alloc(dll);
  d->data = c;
  d->prev = B->cursor->prev;
  d->next = B->cursor;
  B->cursor->prev->next = d;
  B->cursor->prev = d;
}

char tbuf_delete(tbuf* B)
//@requires is_tbuf(B) && !tbuf_at_left(B);
//@ensures is_tbuf(B);
{
  char c = B->cursor->prev->data;
  B->cursor->prev->prev->next = B->cursor;
  B->cursor->prev = B->cursor->prev->prev;
  return c;
}

int tbuf_row(tbuf* B)
//@requires is_tbuf(B);
//@ensures is_tbuf(B);
{
  dll* cur = B->start->next;
  int count_row = 1;
  while (cur != B->cursor) {
    if (char_ord(cur->data) == char_ord('\n')) {
      count_row++;
    }
    cur = cur->next;
  }
  return count_row;
}

int tbuf_col(tbuf* B)
//@requires is_tbuf(B);
//@ensures is_tbuf(B);
{
  dll* cur = B->start->next;
  int count_col = 0;
  while (cur != B->cursor) {
    if (char_ord(cur->data) == char_ord('\n')) {
      count_col = 0;
    }
    else {
      count_col++;
    }
    cur = cur->next;
  }
  return count_col;
}
```



## `editor.c0`

```c
/**************************************************************************/
/*              COPYRIGHT Carnegie Mellon University 2023                 */
/* Do not post this file or any derivative on a public site or repository */
/**************************************************************************/
/*
 * Augment doubly linked lists with row and column information.
 *
 * 15-122 Principles of Imperative Computation */

/*** Interface ***/

typedef struct editor_header editor;
struct editor_header {
  tbuf* buffer;
  int row;
  int col;
};

bool is_editor(editor* E);

editor* editor_new();                  /* Creates a new, empty text buffer */
void editor_forward(editor* E);        /* Moves the cursor forward (right) */
void editor_backward(editor* E);       /* Moves the cursor backward (left) */
void editor_insert(editor* E, char c); /* Insert a character before cursor */
void editor_delete(editor* E);         /* Remove character before cursor   */
void editor_up(editor* E);             /* Moves the cursor up              */
void editor_down(editor* E);           /* Moves the cursor down            */

/*** Implementation ***/
bool is_editor(editor* E)
{
  if (E == NULL) return false;
  return (is_tbuf(E->buffer) && E->row == tbuf_row(E->buffer) 
          && E->col == tbuf_col(E->buffer));
}

editor* editor_new()
//@ensures is_editor(\result);
{
  editor* E = alloc(editor);
  E->buffer = tbuf_new();
  E->row = 1;
  E->col = 0;
  return E;
}

void editor_forward(editor* E)
//@requires is_editor(E);
//@ensures is_editor(E);
{
  if (tbuf_at_right(E->buffer)) return;
  char c = E->buffer->cursor->data;
  tbuf_forward(E->buffer);
  if (char_ord(c) == char_ord('\n')) {
    E->row = E->row + 1;
    E->col = 0;
  }
  else {
    E->col = E->col + 1;
  }
}

void editor_backward(editor* E)
//@requires is_editor(E);
//@ensures is_editor(E);
{
  if (tbuf_at_left(E->buffer)) return;
  tbuf_backward(E->buffer);
  char c = E->buffer->cursor->data;
  if (char_ord(c) == char_ord('\n')) {
    E->row = E->row - 1;
    E->col = tbuf_col(E->buffer);
  }
  else {
    E ->col = E->col - 1;
  }
}

void editor_insert(editor* E, char c)
//@requires is_editor(E);
//@ensures is_editor(E);
{
  tbuf_insert(E->buffer, c);
  if (char_ord(c) == char_ord('\n')) {
    E->row = E->row + 1;
    E->col = 0;
  }
  else {
    E->col = E->col + 1;
  }
}

void editor_delete(editor* E)
//@requires is_editor(E);
//@ensures is_editor(E);
{
  if (tbuf_at_left(E->buffer)) return;
  char c = tbuf_delete(E->buffer);
  if (char_ord(c) == char_ord('\n')) {
    E->row = E->row - 1;
    E->col = tbuf_col(E->buffer);
  }
  else {
    E->col = E->col - 1;
  }
}



// Bonus: Implement these function if you do the bonus task
void editor_up(editor* E) { }
void editor_down(editor* E) { }
```

