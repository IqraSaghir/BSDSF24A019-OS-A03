CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lreadline

SRC = src/main.c src/shell.c src/execute.c src/builtins.c src/history.c

all: bin/myshell bin/psh

bin/myshell: $(SRC) include/shell.h
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o bin/myshell $(LDFLAGS)

bin/psh: bin/myshell
	cp bin/myshell bin/psh

clean:
	rm -f bin/myshell bin/psh

.PHONY: all clean
