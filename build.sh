#!/bin/sh
set -x
rm -f parse.tab.c
bison parse.y
gcc -c -g *.c
gcc -g -o toi -lm *.o #parse.tab.0
set -x
#flex -olex.yy.c lex.l && gcc -g *.c -o toi -lm -lfl
