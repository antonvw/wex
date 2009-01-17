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
PROGRAM=test.v1.0
INCPATH=/usr/local/include/wxExtension
LIBPATH=/usr/local/lib

CC=$(shell wx-config --cxx)

# Warning flags
WFLAGS=-Wall -Wpointer-arith 

# C environment flags, shared by all build dirs
CFLAGS=-DUSE_OTL `wx-config --cflags`

EXTRAFLAGS=-DWXUSINGDLL -INONE -I/usr/include/cppunit/ -I/usr/local/include/wxExtension/ -O2

LIBS=`wx-config --libs all --debug=no` -lwxextension.v1.0.gtk -lodbc -lcppunit

OBJS=	test.o
#OBJS=	main.o
$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $(OBJS)

# General rule.
%.o:	%.cpp ../include/wx/extension/extension.h
	$(CC) $(WFLAGS) $(EXTRAFLAGS) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROGRAM)

install:
	@cp $(PROGRAM) $(BINPATH)
