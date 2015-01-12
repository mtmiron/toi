#!/bin/sh


CC=gcc
LIBS=-lm
FLAGS=-g -w -O2

SRC_FILES=$(wildcard *.c) parse.tab.c
OBJ_FILES=$(addsuffix .o,$(basename $(SRC_FILES)))


all: toi


parse.tab.c: parse.y
	bison parse.y

%.o: %.c
	$(CC) $(FLAGS) -c $^

toi: $(OBJ_FILES)
	$(CC) $(FLAGS) $^ $(LIBS) -o toi
