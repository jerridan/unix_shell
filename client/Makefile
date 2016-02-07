CC      = gcc
CFLAGS  = -g -O0 -Wall -Werror -std=gnu11
LDFLAGS = -L.

.PHONY: all clean

all: client

client: client.o
	$(CC) -o $@ $^ $(LDFLAGS)

cient.o: client.c client.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a client
