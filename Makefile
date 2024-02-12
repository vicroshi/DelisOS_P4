CFLAGS= -g -Wall -Iinclude
CC=gcc
MOD=./modules
SRC=./source
OBJ= $(patsubst %.c,%.o,$(wildcard $(SRC)/*.c))  $(patsubst %.c,%.o,$(wildcard $(MOD)/*.c))
EXEC = cmpcats
all: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $^

testSimple:
	./$(EXEC) -d dirA dirB -s dirM

testLinks1:
	./$(EXEC) -d dirC dirD -s dirM

testLinks2:
	./$(EXEC) -d dirE dirF -s dirM

testHardLinks1:
	./$(EXEC) -d dirHardA dirHardB -s dirM

testHardLinks2:
	./$(EXEC) -d dirHardC dirHardD -s dirM

clean_merge:
	rm dirM

clean:
	rm $(EXEC) $(OBJ)