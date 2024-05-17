CC = gcc
CFLAGS = -g -Wall -Wextra -fsanitize=address -fsanitize=leak

PROJECT = kry
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

.PHONY: all clean

all: $(PROJECT)

$(PROJECT): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(PROJECT) $(OBJ)