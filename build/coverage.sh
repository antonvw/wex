#!/bin/sh
################################################################################
# Name:      coverage.sh
# Purpose:   Coverage file (for wxExtension)
# Author:    Anton van Wezenbeek
# Copyright: (c) 2012 Anton van Wezenbeek
################################################################################

# Run this file in the build folder

export CPPFLAGS="-fprofile-arcs -ftest-coverage"
export LDFLAGS="-fprofile-arcs -ftest-coverage"

make clean
make

./test-all.sh

lcov --directory gccgtk2_dll --capture --output-file app.info
genhtml app.info
