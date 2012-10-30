#!/bin/sh
bison -v parse.y && gcc -g -o toi -lm $* *.c
#flex -olex.yy.c lex.l && gcc -g *.c -o toi -lm -lfl
