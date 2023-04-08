GCC = gcc
GDB = gdb
GCCFLAGS = -Wall -Werror=format-security -Werror=implicit-function-declaration
TARGET = ./bin/cqlite
DEBUG_TARGET = ./bin/cqlite.dbg
SRC = main.c statement.c metacmd.c reader.c

all:
	$(GCC) -o $(TARGET) $(GCCFLAGS) $(SRC)
	$(TARGET)

.PHONY: debug
debug:
	$(GCC) -g -o $(TARGET) $(GCCFLAGS) $(SRC)
	$(GDB) -q $(TARGET)
