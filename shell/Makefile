CC      = gcc
CFLAGS  = -g -O0 -Wall -Werror -std=gnu11
LDFLAGS = -L.

.PHONY: all clean

all: shell

shell: shell.o error_handling.o piping.o execution.o io_redirection.o
	$(CC) -o $@ $^ $(LDFLAGS)

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -o $@ -c $<

error_handling.o: error_handling.c error_handling.h
	$(CC) $(CFLAGS) -o $@ -c $<

piping.o: piping.c piping.h
	$(CC) $(CFLAGS) -o $@ -c $<

execution.o: execution.c execution.h
	$(CC) $(CFLAGS) -o $@ -c $<

io_redirection.o: io_redirection.c io_redirection.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a shell
