Text Editor in C
===============================================================================
RYe's editor

Using RYe's editor:
   % make rye
   % ./rye <optional file name>

Key bindings:
   ^Q — quit
   ^S — save
   ^A — go to start of line
   ^E — go to end of line
   ^W — page up
   ^D — page down
   ^F — search (not implemented yet)
   ^O — open file (not implemented yet)
   arrow keys — move cursor

Checking for memory leaks:
   % make rye
   % leaks --atExit -- ./rye
===============================================================================
Kilo editor

Using kilo editor:
  % make kilo
  % ./kilo <optional file name>
===============================================================================
Escape sequence / ANSI code

Compiling escape sequence printing:
  % make escape
  % ./escape
===============================================================================
Gap buffer interface

Testing gap buffer interface with contracts:
   % gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
   % leaks --atExit -- ./a.out

Testing gap buffer without contracts:
   % gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
   % leaks --atExit -- ./a.out
===============================================================================
Editor interface

Testing editor with contracts:
   % gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
   % leaks --atExit -- ./a.out

Testing editor without contracts:
   % gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
   % leaks --atExit -- ./a.out
===============================================================================
