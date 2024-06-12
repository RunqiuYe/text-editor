#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "gapbuf.h"
#include "editor.h"

int main(void) {
  printf("Testing editor library...");

  editor* A = editor_new();
  assert(is_editor(A));
  assert(A->row == 1);
  assert(A->col == 0);
  assert(A->numrows == 1);

  editor_insert(A, 'a');
  editor_insert(A, 'p');
  editor_insert(A, 'l');
  editor_insert(A, 'l');
  editor_insert(A, 'e'); // apple[]
  assert(A->row == 1);
  assert(A->col == 5);
  assert(A->numrows == 1);

  editor_insert(A, '\n'); // apple\n[]
  assert(A->row == 2);
  assert(A->col == 0);
  assert(A->numrows == 2);

  editor_insert(A, 'p');
  editor_insert(A, 'i');
  editor_insert(A, 'e'); // applepie[]
  assert(A->row == 2);
  assert(A->col == 3);
  assert(A->numrows == 2);

  editor_forward(A); // applepie[]
  assert(A->row == 2);
  assert(A->col == 3);
  assert(A->numrows == 2);

  editor_backward(A);
  editor_backward(A); // apple\np[]ie
  assert(A->row == 2);
  assert(A->col == 1);
  assert(A->numrows == 2);

  editor_backward(A); // apple\n[]pie
  assert(A->row == 2);
  assert(A->col == 0);
  assert(A->numrows == 2);

  editor_backward(A); // apple[]\npie
  assert(A->row == 1);
  assert(A->col == 5);
  assert(A->numrows == 2);

  editor_insert(A, 'A');
  editor_insert(A, 'P');
  editor_insert(A, 'P');
  editor_insert(A, 'L');
  editor_insert(A, 'E'); // appleAPPLE[]\npie
  assert(A->row == 1);
  assert(A->col == 10);
  assert(A->numrows == 2);

  editor_backward(A);
  editor_backward(A);
  editor_backward(A);
  editor_backward(A);
  editor_backward(A);
  editor_insert(A, '\n'); // apple\n[]APPLE\npie
  assert(A->row == 2);
  assert(A->col == 0);
  assert(A->numrows == 3);

  editor_insert(A, 'P');
  editor_insert(A, 'I');
  editor_insert(A, 'E'); // apple\nPIE[]APPLE\npie
  assert(A->row == 2);
  assert(A->col == 3);
  assert(A->numrows == 3);

  editor_free(A);

  printf("Passed all tests!");

  return 0;
}
