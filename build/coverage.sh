#!/bin/sh
################################################################################
# Name:      coverage.sh
# Purpose:   Coverage file (for wxExtension)
# Author:    Anton van Wezenbeek
# Copyright: (c) 2012 Anton van Wezenbeek
################################################################################

# Run this file in the build folder

export CPPFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
export LDFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"

lcov --capture --initial --directory gccgtk2_dll --output-file app.base

make clean
make

./test-all.sh

lcov --capture --directory gccgtk2_dll --output-file app.run

# remove output for external libraries
lcov --remove app.run "/usr*" --output-file app.run

lcov --add-tracefile app.base --add-tracefile app.run --output-file app.total

genhtml app.total

rm -f app.base app.run app.total
