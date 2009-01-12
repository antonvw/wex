# File:          Makefile
# Author:        Anton van Wezenbeek
# RCS-ID:        $Id$
# Purpose:       Makefile for wxextension sample gtk

LIBS=-L../libs -lwxextension.v1.0.gtk `wx-config --libs all` -lodbc
CFLAGS=`wx-config --cflags`
APP_VERSION=sample
PROGRAM=$(APP_VERSION).gtk

CC=$(shell wx-config --cxx)

# Warning flags
WFLAGS=-Wall -Wpointer-arith

# C environment flags, shared by all build dirs
# Set tracing using DWXTRACE, and trace string (only in debug mode).
EXTRAFLAGS=-DWXUSINGDLL -INONE -I../include -O2

OBJS=	appl.o
$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $(OBJS)

# General rule.
%.o: %.cpp
	$(CC) $(WFLAGS) $(EXTRAFLAGS) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROGRAM)
