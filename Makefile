rye: gapbuf.c editor.c window.c main.c
	$ gcc -o rye -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c window.c main.c