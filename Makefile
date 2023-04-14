GCC = gcc
GDB = gdb
GCCFLAGS = -Wall -Werror=format-security -Werror=implicit-function-declaration
TARGET = ./bin/cqlite
DEBUG_TARGET = ./bin/cqlite.dbg
SRC = main.c statement.c metacmd.c reader.c leaf.c constants.c

all: build
	$(TARGET)

.PHONY: build
build:
	$(GCC) -o $(TARGET) $(GCCFLAGS) $(SRC)

.PHONY: debug
debug:
	$(GCC) -g -o $(TARGET) $(GCCFLAGS) $(SRC)
	$(GDB) -q $(TARGET)

.PHONY: tests
tests: build
	python -m unittest test_cqlite.py
