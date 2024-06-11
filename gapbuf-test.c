#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"

int main(void) {
  printf("%s", "Testing gap buffer library...\n");

  char *s = "apple";

  // basics tests
  gapbuf* A = gapbuf_new(10);
  assert(is_gapbuf(A));
  assert(gapbuf_at_left(A));
  assert(gapbuf_at_right(A));
  assert(gapbuf_row(A) == 1);
  assert(gapbuf_col(A) == 0);
  // gapbuf_print(A); // empty gap buffer

  gapbuf_insert(A, 'a');
  gapbuf_insert(A, 'p');
  gapbuf_insert(A, 'p');
  gapbuf_insert(A, 'l');
  gapbuf_insert(A, 'e');
  assert(is_gapbuf(A));
  assert(!gapbuf_at_left(A));
  assert(gapbuf_at_right(A));
  assert(gapbuf_row(A) == 1);
  assert(gapbuf_col(A) == 5);
  // gapbuf_print(A); // apple[]

  char* s1 = gapbuf_str(A);
  assert(strcmp(s1, s) == 0);
  free(s1);
  
  gapbuf_backward(A);
  assert(is_gapbuf(A));
  assert(!gapbuf_at_left(A));
  assert(!gapbuf_at_right(A));
  assert(gapbuf_row(A) == 1);
  assert(gapbuf_col(A) == 4);
  // gapbuf_print(A); // appl[]e
  
  char* s2 = gapbuf_str(A);
  assert(strcmp(s2, s) == 0);
  free(s2);

  gapbuf_backward(A);
  gapbuf_backward(A);
  gapbuf_backward(A);
  gapbuf_backward(A);
  assert(is_gapbuf(A));
  assert(gapbuf_at_left(A));
  assert(!gapbuf_at_right(A));
  assert(gapbuf_row(A) == 1);
  assert(gapbuf_col(A) == 0);
  gapbuf_print(A); // []apple

  char* s3 = gapbuf_str(A);
  assert(strcmp(s3, s) == 0);
  free(s3);

  gapbuf_free(A);

  printf("Basics tests passed!\n");

  // test for resizing
  printf("Testing for array resize...\n");

  gapbuf* B = gapbuf_new(3);
  assert(is_gapbuf(B));
  assert(gapbuf_at_left(B));
  assert(gapbuf_at_right(B));
  assert(gapbuf_row(B) == 1);
  assert(gapbuf_col(B) == 0);
  // gapbuf_print(B); // empty gap buffer

  gapbuf_insert(B, 'a');
  gapbuf_insert(B, 'p');
  gapbuf_insert(B, 'p');
  gapbuf_insert(B, 'l');
  gapbuf_insert(B, 'e');
  assert(is_gapbuf(B));
  assert(!gapbuf_at_left(B));
  assert(gapbuf_at_right(B));
  assert(gapbuf_row(B) == 1);
  assert(gapbuf_col(B) == 5);
  // gapbuf_print(B); // apple[]

  char* s5 = gapbuf_str(B);
  assert(strcmp(s5, s) == 0);
  free(s5);

  gapbuf_free(B);

  // special case: start with array size 1
  gapbuf* C = gapbuf_new(1);
  assert(is_gapbuf(C));
  assert(gapbuf_at_left(C));
  assert(gapbuf_at_right(C));
  gapbuf_insert(C, 'A');
  gapbuf_insert(C, 'P');
  gapbuf_insert(C, 'P');
  gapbuf_insert(C, 'L');
  gapbuf_insert(C, 'E');
  gapbuf_insert(C, '\n');
  assert(is_gapbuf(C));
  assert(!gapbuf_at_left(C));
  assert(gapbuf_at_right(C));
  assert(gapbuf_row(C) == 2);
  assert(gapbuf_col(C) == 0);
  assert(gapbuf_numrows(C) == 2);
  // gapbuf_print(C); // APPLE\n[]
  gapbuf_backward(C);
  assert(gapbuf_row(C) == 1);
  assert(gapbuf_col(C) == 5);

  gapbuf_forward(C);
  gapbuf_insert(C, 'A');
  gapbuf_insert(C, '\n');
  gapbuf_insert(C, 'P');
  gapbuf_insert(C, '\n');
  assert(is_gapbuf(C));
  assert(!gapbuf_at_left(C));
  assert(gapbuf_at_right(C));
  assert(gapbuf_row(C) == 4);
  assert(gapbuf_col(C) == 0);
  assert(gapbuf_numrows(C) == 4);
  gapbuf_print(C); // APPLE\nA\nP\n[]

  gapbuf_free(C);

  printf("All test cases passed!\n");
}
