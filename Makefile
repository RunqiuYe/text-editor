rye: src/gapbuf.c src/editor.c src/window.c src/main.c
	$ gcc -o rye -O3 -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic src/gapbuf.c src/editor.c src/window.c src/main.c
kilo: src/kilo.c
	$ gcc -o kilo -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic src/kilo.c
escape: src/escape.c
	$ gcc -o escape -fsanitize=undefined -g -Wall -Wextra -Werror -Wshadow -std=c99 -pedantic src/escape.c
