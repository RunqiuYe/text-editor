rye: gapbuf.c editor.c window.c main.c
	$ gcc -o rye -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic gapbuf.c editor.c window.c main.c
kilo: test.c
	$ gcc -o kilo -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic test.c
escape: escape.c
	$ gcc -o escape -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic escape.c
