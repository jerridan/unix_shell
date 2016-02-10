CC      = gcc
CFLAGS  = -g -O0 -Wall -Werror -std=gnu11
LDFLAGS = -L.

.PHONY: all clean

all: client

client: client.o error_handling.o piping.o execution.o
	$(CC) -o $@ $^ $(LDFLAGS)

client.o: client.c client.h
	$(CC) $(CFLAGS) -o $@ -c $<

error_handling.o: error_handling.c error_handling.h
	$(CC) $(CFLAGS) -o $@ -c $<

piping.o: piping.c piping.h
	$(CC) $(CFLAGS) -o $@ -c $<

execution.o: execution.c execution.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a client
