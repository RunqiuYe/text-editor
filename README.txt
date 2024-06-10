Text Editor in C

===============================================================================

Gap buffer

Compiling C code with contracts:
   % gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
   % ./a.out

Compiling C code without contracts:
   % gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
   % ./a.out

===============================================================================

Kilo editor

Compiling kilo text editor:
  % gcc -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic test.c
  % ./a.out

Compiling escape sequence printing:
  % gcc -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic escape.c
  % ./a.out

