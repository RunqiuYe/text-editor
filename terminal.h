#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include "gapbuf.h"
#include "editor.h"

void die(const char* s);

void enableRawMode(editor* E);
void disableRawMode(editor* E);
