#-------------------------------------------------------------------------------
# File:    Makefile.env
# Purpose: Makefile for cpp unit testing
# Author:  Anton van Wezenbeek                                    
# RCS-ID:  $Id$
#                                                                               
# Copyright  2009, Anton van Wezenbeek
# All rights are reserved. Reproduction in whole or part is prohibited
# without the written consent of the copyright owner.
#-------------------------------------------------------------------------------

WXWIN_BASE_VERSION=$(shell wx-config --release)
LIBS=-L../../lib -lwxext.v1.0.gtk -lwxextrep.v1.0.gtk `wx-config --libs all` -lodbc -lcppunit
CFLAGS=-DUSE_OTL `wx-config --cflags`
LIBPATH=/usr/local/lib
PROGRAM=test.v1.0

CC=$(shell wx-config --cxx)

# Warning flags
WFLAGS=-Wall -Wpointer-arith 

EXTRAFLAGS=-DWXUSINGDLL -INONE -I/usr/include/cppunit/ -I../../include -O2

OBJS=	test.o\
	main.o
$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $(OBJS)

# General rule.
%.o:	%.cpp
	$(CC) $(WFLAGS) $(EXTRAFLAGS) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROGRAM)

install:

run:
	./$(PROGRAM)
