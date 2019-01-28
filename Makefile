CC=gcc
CFLAGS=-std=gnu99 -Wall -Werror -Os
LFLAGS=-L../ZDK
INCLUDES=-I../ZDK
LIBS=-lzdk -lncurses -lm

pong: graphics_utils.c graphics_utils.h rng.c rng.h utils.c utils.h pong.c pong.h
	$(CC) $(CFLAGS) $(INCLUDES) -o pong *.c $(LFLAGS) $(LIBS)

clean:
	rm pong
