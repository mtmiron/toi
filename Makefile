#!/bin/sh


CC=gcc
LIBS=-lm

SRC_FILES=$(wildcard *.c) parse.tab.c
OBJ_FILES=$(addsuffix .o,$(basename $(SRC_FILES)))


all: toi


parse.tab.c: parse.y
	bison parse.y

%.o: %.c
	$(CC) -g -c $^

toi: $(OBJ_FILES)
	$(CC) $^ $(LIBS) -g -o toi
