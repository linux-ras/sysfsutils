# System utilities makefile

CC=gcc

CFLAGS=-g -Wall
INCLUDE=../include
LIB=../lib

RM=rm -f 

all: lsbus systool

lsbus: lsbus.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o lsbus lsbus.c -lsysfs -L$(LIB)

systool: systool.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o systool systool.c -lsysfs -L$(LIB)

clean:
	$(RM) *.o *~ lsbus systool
