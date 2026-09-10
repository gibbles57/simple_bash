CC = gcc -std=gnu99
OPTS = -Wall -Werror -pedantic-errors
DEBUG = -g

SRCDIR = ./src
OBJDIR = ./obj
INCDIR = ./inc

INCLUDE=$(addprefix -I,$(INCDIR))
CFLAGS=$(OPTS) $(INCLUDE) $(DEBUG)
OBJECTS=$(addprefix $(OBJDIR)/,cs262sh.o exec.o)

all: cs262sh

cs262sh: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
$(OBJDIR)/cs262sh.o: $(SRCDIR)/cs262sh.c $(INCDIR)/exec.h $(INCDIR)/listnode.h
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm cs262sh
	rm $(OBJDIR)/cs262sh.o
