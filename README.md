# Text Editor in C
## RYe's editor

Using RYe's editor:

```
% cd src
% make rye
% ./rye <file names (optional)>
```

**Note:** If some input filename does not exist, new file with that name will be created.

Key bindings:

```
^Q — quit
^O — open file
^J — go to previous open file
^K — go to next open file
^S — save
^X — close file
^A — go to start of line
^E — go to end of line
^W — page up
^D — page down
^F — search
arrow keys — move cursor
```

Checking for memory leaks:

```
% cd src
% make rye
% leaks --atExit -- ./rye
```

## Kilo editor

Using kilo editor:

```
% cd src
% make kilo
% ./kilo <file names (optional)>
```


## Escape sequence / ANSI code

Compiling escape sequence printing:

```
% cd src
% make escape
% ./escape
```


## Gap buffer interface

Testing gap buffer interface with contracts:

```
% gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
% leaks --atExit -- ./a.out
```

Testing gap buffer without contracts:

```
% gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
% leaks --atExit -- ./a.out
```


## Editor interface

Testing editor with contracts:

```
% gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
% leaks --atExit -- ./a.out
```

Testing editor without contracts:

```
% gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
% leaks --atExit -- ./a.out
```
