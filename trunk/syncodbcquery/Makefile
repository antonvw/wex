# File:          Makefile
# Author:        Anton van Wezenbeek
# RCS-ID:        $Id: Makefile 104 2008-12-03 13:36:59Z antonvw $
# Purpose:       Makefile for syncodbcquery gtk

LIBS=`wx-config --libs all` -lwxextension.v2.0.gtk -lwxfiletool.v2.0.gtk -lodbc
CFLAGS=`wx-config --cflags`
APP_VERSION=syncodbcquery
PROGRAM=$(APP_VERSION).gtk

CC=$(shell wx-config --cxx)

# Warning flags
WFLAGS=-Wall -Wpointer-arith

# C environment flags, shared by all build dirs
# Set tracing using DWXTRACE, and trace string (only in debug mode).
EXTRAFLAGS=-DWXUSINGDLL -INONE -I/usr/local/include/wxextension -I/usr/local/include/wxfiletool -O2

OBJS=	appl.o
$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $(OBJS)

# General rule.
%.o: src/%.cpp
	$(CC) $(WFLAGS) $(EXTRAFLAGS) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROGRAM)
