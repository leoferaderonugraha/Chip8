# Minimal make file

CC=g++
CFLAGS=-lSDL2 -I/usr/include/SDL2 -D_REENTRANT

main:
	$(CC) $@.cpp Chip8.cpp -o chip8 $(CFLAGS)