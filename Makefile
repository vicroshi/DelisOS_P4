CFLAGS=  -g -Wall -Iinclude
CC=gcc
MOD=./modules
SRC=./source
OBJ=$(patsubst %.c,%.o,$(wildcard $(MOD)/*.c))
all:$(SRC)/cmpcats.c $(OBJ)
	$(CC) $(CFLAGS) -o cmpcats $^
	
clean:
	rm cmpcats $(OBJ)