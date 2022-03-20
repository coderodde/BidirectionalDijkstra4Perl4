all: *.c
	gcc -O3 -ansi -pedantic -Wall -Werror -fmax-errors=1 *.c
