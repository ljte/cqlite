GCC = gcc
GCCFLAGS = -Wall -Werror=format-security -Werror=implicit-function-declaration
TARGET = ./bin/cqlite
SRC = main.c
all:
	$(GCC) -o $(TARGET) $(GCCFLAGS) $(SRC)
	$(TARGET)

