rye: src/gapbuf.c src/editor.c src/window.c src/main.c
	$ clang -o src/rye -O3 -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic src/gapbuf.c src/editor.c src/window.c src/main.c
kilo: test.c
	$ clang -o kilo -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic test.c
escape: escape.c
	$ clang -o escape -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic escape.c
