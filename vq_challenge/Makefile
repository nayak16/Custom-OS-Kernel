# This Makefile is for building and testing under Linux.
# Once everything works there, you probably want to move
# variable_queue.h into kern/inc and/or user/inc.

TEST=vqtest
CC=gcc
CFLAGS = -g -fno-strict-aliasing -Wall -gdwarf-2 -Werror -m32

all: $(TEST)

$(TEST): $(TEST).c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(TEST)
