Text Editor in C

===============================================================================

Kilo editor

Compiling kilo text editor:
  % gcc -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic test.c
  % ./a.out

Compiling escape sequence printing:
  % gcc -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic escape.c
  % ./a.out

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
   % gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c terminal.c main.c
   % ./a.out

Compiling C code without contracts:
   % gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
   % leaks --atExit -- ./a.out
