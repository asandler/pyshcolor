all:
	gcc pyshcolor.c -o pyshcolor -D _BSD_SOURCE -std=c99 -lncurses
