# Text editor in C

## Gap buffer

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
gb->back =  'e'  'i'  'p'  '\0' '\0' '\0' '\0' '\0'
gb->frontlen = 5
gb->backlen = 3
gb->limit = 8
```

Moving the cursor forward, we will have `applep[]ie`, and the content will become

```c
gb->front = 'a'  'p'  'p'   'l'  'e'  'p'  '\0' '\0'
gb->back =  'e'  'i'  '\0'  '\0' '\0' '\0' '\0' '\0'
gb->frontlen = 6
gb->backlen = 2
gb->limit = 8
```

Since we have the length of both strings recorded in `frontlen` and `backlen`, we only need to change the end of `back` to `'\0'` (which is the start of the string we read after the cursor) and the end of `front` to the corresponding characters.

Inserting `L` at the cursor position, we will have `applepL[]ie` and the content will become

```c
gb->front = 'a'  'p'  'p'   'l'  'e'  'p'  'L' '\0'
gb->back =  'e'  'i'  '\0'  '\0' '\0' '\0' '\0' '\0'
gb->frontlen = 7
gb->backlen = 2
gb->limit = 8
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
void gapbuf_insert(gapbuf* gb);             // insert a character before cursor
char gapbuf_delete(gapbuf* gb);             // delete a character before cursor and return deleted char

int gapbuf_row(gapbuf* gb);                 // return row of cursor
int gapbuf_col(gapbuf* gb);                 // return column of cursor

char* gapbuf_free(gapbuf* gb);              // free allocated gapbuffer
// char* gapbuf_str(gapbuf* gb);
```



