# Text editor in C

## Structure

### Gap buffer

We will use *gap buffer* to store contents of an editor. In the gap buffer, the position of the cursor is represented with a gap. In other word, the gap is the cursor. With the help of this gap, we can achieve efficient movement of the cursor, insertion, and deletion of text, as will be shown. To illustrate, consider the following text `applepie` with cursor to the right of the first `e`. With gap buffer, this can be represented as `apple[]pie`.

Starting from this gap buffer, we can have a series of operations:

- `applep[]ie` — hit the right key to move the cursor forward
- `applepi[]e` — hit the right key to move the cursor forward
- `applepie[]` — hit the right key to move the cursor forward
- `applepi[]` — hit delete
- `applepiE[]` — type `E` to insert "E"
- `[]applepiE` — hit the left key 8 times to move the cursor to the left most position

We use two strings to implement the gap buffer — one string before the gap and another *inverted* string after the gap. The string after the gap needs to be inverted in order to make insertion/deletion/moving more efficient. First of all, gap buffer, which have type `gapbuf`, is defined as follows:

```c
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
```

To illustrate, consider for example a gap buffer `gapbuf* gb` which holds the text `apple[]pie`. Here, strings are represented as an array of chars, which is of course also the string implementation in C. The content of `gb` will be as such.

```c
gb->front = 'a'  'p'  'p'  'l'  'e'  '\0' '\0' '\0'
gb->back  = 'e'  'i'  'p'  '\0' '\0' '\0' '\0' '\0'
gb->frontlen = 5
gb->backlen  = 3
gb->limit    = 8
```

Moving the cursor forward, we will have `applep[]ie`, and the content will become

```c
gb->front = 'a'  'p'  'p'   'l'  'e'  'p'  '\0' '\0'
gb->back  = 'e'  'i'  '\0'  '\0' '\0' '\0' '\0' '\0'
gb->frontlen = 6
gb->backlen  = 2
gb->limit    = 8
```

Since we have the length of both strings recorded in `frontlen` and `backlen`, we only need to change the end of `back` to `'\0'` (which is the start of the string we read after the cursor) and the end of `front` to the corresponding characters.

Inserting `L` at the cursor position, we will have `applepL[]ie` and the content will become

```c
gb->front = 'a'  'p'  'p'   'l'  'e'  'p'  'L' '\0'
gb->back  = 'e'  'i'  '\0'  '\0' '\0' '\0' '\0' '\0'
gb->frontlen = 7
gb->backlen  = 2
gb->limit    = 8
```

Notice that the `front` buffer is now full, we double the length of it. To make the `back` buffer also having the same length, we also need to double the length of the back buffer. In this way, insertions, deletions, and cursor movement will all have `O(1)` amortized cost, achieving great efficiency.

The gap buffer library will have the following functions.

```c
bool is_gapbuf(gapbuf* gb);                 // representation invariant

bool gapbuf_at_left(gapbuf* gb);            // return true if cursor(gap) is at leftmost position
bool gapbuf_at_right(gapbuf* gb);           // return true if cursor(gap) is at rightmost position

gapbuf* gapbuf_new(size_t init_limit);      // create new empty gap buffer
void gapbuf_forward(gapbuf* gb);            // move the cursor forward (to the right)
void gapbuf_backward(gapbuf* gb);           // move the cursor backward (to the left)
void gapbuf_insert(gapbuf* gb, char c);     // insert a character before cursor
char gapbuf_delete(gapbuf* gb);             // delete a character before cursor and return deleted char
char gapbuf_delete_right(gapbuf* gb);       // delete a character after cursor and return deleted char

// void strbuf_add(strbuf *sb, char *str, size_t len);
// void strbuf_addstr(strbuf *sb, char *str);

size_t gapbuf_row(gapbuf* gb);                 // row of cursor position
size_t gapbuf_col(gapbuf* gb);                 // column of cursor position
size_t gapbuf_numrows(gapbuf* gb);             // number of rows in gap buffer

void gapbuf_free(gapbuf* gb);              // free allocated gapbuffer, and return the string contained
char* gapbuf_str(gapbuf* gb);               // the string contained in the text buffer
void gapbuf_print(gapbuf* gb);              // print the content of gapbuf for debugging

```

### Editor

For each text file, we want to use an editor to modify the file. 

We want to use the gap buffer to hold the text in the editor. Since we want to show the current location of cursor on the screen, we need to include the row and the column of the cursor in the text, together with the total number of rows. Hence we define the editor struct as the following for now.

```c
struct editor_header {
  gapbuf* buffer;
  size_t row;
  size_t col;
  size_t numrows;
};
typedef struct editor_header editor;
```

 Based on the description, the representation invariant should ensures `buffer` is a gap buffer. The `row`, `column`, and `numrows` field should also show the correct value. That is,

```c 
bool is_editor(editor* E) {
  if (E == NULL) return false;
  if (!is_gapbuf(E->buffer)) return false;
  if (E->row != gapbuf_row(E->buffer)) return false;
  if (E->col != gapbuf_col(E->buffer)) return false;
  if (E->numrows != gapbuf_numrows(E->buffer)) return false;
  return true;
}
```

where we extend the `gapbuf` interface with a new function `gapbuf_numrows`. The editor interface contains the following functions.

```c
bool is_editor(editor* E);              // representation invariant

editor* editor_new(void); 	            // create a new and empty editor
void editor_forward(editor* E); 	      // move the cursor forward, to the right
void editor_backward(editor* E); 	      // move the cursor backward, to the left
void editor_insert(editor* E, char c); 	// insert c to the cursor’s left
void editor_delete(editor* E); 	        // remove the node to the cursor’s left

void editor_free(editor* E);            // free allocated space for editor
```

Note that the user will interact with the editor directly. Therefore, when the cursor is at the rightmost of the editor, moving forward should keep the editor unchanged instead of raising and error. The same is true for moving backward and delete.

Now we finish the part where the editor interacts with the buffer (text). Next we want to add how the editor interacts with our terminal, since that's where the text editor is going to be.

### Window

We use a new structure `window` to directly interacts with terminal and display our file. This will make things easier when we allow the text editor to open multiple buffers at a time. In that case, we only have one window, but we can have multiple editor, one for each file.

The `window` struct must hold an editor to know which file it is displaying. Hence, it must have an editor field. In addition, we store the original terminal in the struct, together with total number of rows and columns on the screen.

```c
struct window_header {
  editor* editor;
  struct termios orig_terminal;
  size_t screenrows;                // total number of rows on screen
  size_t screencols;                // total number of cols on screen
};
typedef struct window_header window;
```

We also update the `editor` interface. We add two fields `rowoff` and `coloff` with type `size_t`. This will allow us to render the text more easily and eventually scroll with our arrow keys. We also include the name of the current file, so as to render it on our status bar. Now the `editor` type looks like this.

```c
struct editor_header {
  gapbuf* buffer;
  size_t row;
  size_t col;
  size_t numrows;

  size_t rowoff;        // first visible row
  size_t coloff;        // first visible col
  
  char* filename;
};
typedef struct editor_header editor;
```

The `window` library contains these functions. This will allow us to initialize the window, enable raw mode, display everything, process key press, and open text files.

```c

void die(window* W, const char* s);               // debugging and display error

window* window_new(void);                         // create new window

void enableRawMode(window* W);                    // enable raw mode
void disableRawMode(window* W);                   // disable raw mode

int getCursorPosition(size_t* row, size_t* col);
int getScreenSize(size_t* numrows, size_t* numcols);
void getWindowSize(window* W);

void scroll(window* W);                           // adjust offset to scroll
void refresh(window* W);                          // redraw everything
void renderStatusBar(window* W);                  // draw status bar
void renderText(window* W);                       // render text file
void renderMessageBar(window* W);                 // render message bar
void setMessage(window* W, const char* fmt, ...); // set message bar message
void render(window* W);

int readKey(window* W);                           // read key presses
void moveCursor(window* W, int key);              // move cursor
void processKey(window* W, bool* go);             // process key press

void openFile(window* W, char* filename);         // open text file
void saveFile(window* W);                         // save edited file

void window_free(window* W);                      // free
```

Accordingly, we have a rough model of our `main` functino in `main.c`. We first initialize the window, then enable raw mode for our text editor. The command line arguments allow us to open files when opening the editor. This finishes the setup. After that, we constantly read in key presses from user and refresh the screen accordingly. When the user quite the editor, we disable raw mode and set the terminal to it's original setup, free all allocated memory, and end the program.

```c
int main(int argc, char* argv[]) {
  window* W = window_new();
  enableRawMode(W);
  if (argc >= 2) {
    openFile(W, argv[1]);
  }

  bool* go = xcalloc(1, sizeof(bool));
  *go = true;

  while (*go) {
    refresh(W);
    processKey(W, go);
  }

  disableRawMode(W);
  window_free(W);
  free(go);
  printf("Thanks for using RYe's editor\n");
  return 0;
}
```



## Implementation





## References

[Build your own text editor](https://viewsourcecode.org/snaptoken/kilo/)

[ANSI escape code](https://en.wikipedia.org/wiki/ANSI_escape_code#)

