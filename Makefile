#!/bin/sh


CC=gcc
LIBS=-lm
FLAGS=-w -O0

SRC_FILES=$(wildcard *.c) parse.tab.c
OBJ_FILES=$(addsuffix .o,$(basename $(SRC_FILES)))


all: toi

clean:
	rm -f toi *.o

#
# Bison 3.0+ broke backwards compatibility with 2.7
#
##parse.tab.c: parse.y
#	bison parse.y

%.o: %.c
	$(CC) $(FLAGS) -c $^

toi: $(OBJ_FILES)
	$(CC) $(FLAGS) $^ $(LIBS) -o toi
