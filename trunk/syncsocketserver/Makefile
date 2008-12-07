# File:          Makefile
# Author:        Anton van Wezenbeek
# RCS-ID:        $Id$
# Purpose:       Makefile for syncsocketserver gtk

LIBS=`wx-config --libs all` -lwxextension.v2.0.gtk -lwxfiletool.v2.0.gtk -lodbc
CFLAGS=`wx-config --cflags`
APP_VERSION=syncsocketserver
PROGRAM=$(APP_VERSION).gtk

CC=$(shell wx-config --cxx)

# Warning flags
WFLAGS=-Wall -Wpointer-arith

# C environment flags, shared by all build dirs
# Set tracing using DWXTRACE, and trace string (only in debug mode).
EXTRAFLAGS=-DWXUSINGDLL -INONE -I/usr/local/include/wxextension -I/usr/local/include/wxfiletool -O2

OBJS=	server.o
$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $(OBJS)

# General rule.
%.o: src/%.cpp
	$(CC) $(WFLAGS) $(EXTRAFLAGS) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROGRAM)
