# File:          Makefile
# Author:        Anton van Wezenbeek
# RCS-ID:        $Id$
# Purpose:       Makefile for syncodbcquery gtk

LIBS=-L../extension/lib -lwxext.v1.0.gtk -lwxextrep.v1.0.gtk `wx-config --libs all` -lodbc
BINPATH=/usr/local/bin
CFLAGS=`wx-config --cflags`
APP_VERSION=syncodbcquery.v1.0
PROGRAM=$(APP_VERSION).gtk

CC=$(shell wx-config --cxx)

# Warning flags
WFLAGS=-Wall -Wpointer-arith

# C environment flags, shared by all build dirs
# Set tracing using DWXTRACE, and trace string (only in debug mode).
EXTRAFLAGS=-DWXUSINGDLL -INONE -I../extension/include -O2

OBJS=	appl.o
$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $(OBJS)

# General rule.
%.o: src/%.cpp src/appl.xpm
	$(CC) $(WFLAGS) $(EXTRAFLAGS) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROGRAM)

install:
	@cp $(PROGRAM) $(BINPATH)
