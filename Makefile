CFLAGS= -Iinclude
CC=gcc
SRC=./source
all:$(SRC)/cmpcats.c
	$(CC) $(CFLAGS) -o cmpcats $^