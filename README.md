# Text Editor in C
## RYe's editor

Using RYe's editor:

```
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

## Kilo editor

Using kilo editor:

```
% make kilo
% ./kilo <file names (optional)>
```


## Escape sequence / ANSI code

Compiling escape sequence printing:

```
% make escape
% ./escape
```


## Gap buffer interface

Testing gap buffer interface with contracts:

```
% cd src
% gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
```

Testing gap buffer without contracts:

```
% cd src
% gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c gapbuf-test.c
```


## Editor interface

Testing editor with contracts:

```
% cd src
% gcc -DDEBUG -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
```

Testing editor without contracts:

```
% cd src
% gcc -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c editor-test.c
```
