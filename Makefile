.PHONY:	all clean

CC = gcc
CFLAGS = -Wall -g -D_FILE_OFFSET_BITS=64

all:	treetest ndstest ndsfuse

treetest: treetest.o tree.o
treetest.o:	treetest.c
tree.o:	tree.c tree.h

ndstest: ndstest.o ndsrom.o tree.o
ndstest.o: ndstest.c tree.h
ndsrom.o: ndsrom.c ndsrom.h tree.h

ndsfuse: ndsfuse.o ndsrom.o tree.o -lfuse
ndsfuse.o: ndsfuse.c ndsrom.h tree.h

clean:
	rm -rf treetest ndstest ndsfuse *.o
