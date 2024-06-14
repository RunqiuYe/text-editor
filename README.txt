Text Editor in C

===============================================================================

Kilo editor

Compiling kilo text editor:
  % make kilo
  % ./kilo <optional file name>

Compiling escape sequence printing:
  % make escape
  % ./escape

===============================================================================

Gap buffer interface

Compiling C code with contracts:
   % gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
   % leaks --atExit -- ./a.out

Compiling C code without contracts:
   % gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
   % leaks --atExit -- ./a.out

===============================================================================

Editor interface

Compiling C code with contracts:
   % gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
   % leaks --atExit -- ./a.out

Compiling C code without contracts:
   % gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
   % leaks --atExit -- ./a.out

===============================================================================

Using RYe's editor

Compiling C code with contracts:
   % gcc -o rye -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c window.c main.c
   % leaks --atExit -- ./rye

Compiling C code without contracts:
   % make rye
   % ./rye <optional file name>

Checking for memory leaks:
   % make rye
   % leaks --atExit -- ./rye
