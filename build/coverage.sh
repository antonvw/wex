#!/bin/sh
################################################################################
# Name:      coverage.sh
# Purpose:   Coverage file (for wxExtension)
# Author:    Anton van Wezenbeek
# Copyright: (c) 2012 Anton van Wezenbeek
################################################################################

# Run this file in the build folder

TESTDIR=./gccgtk2_dll/

export CPPFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
export LDFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"

echo "-- make clean --"
make clean

echo "-- make test coverage build --"
make

echo "-- lcov initializing --"
lcov --base-directory ~/wxExtension/extension --capture --initial --directory $TESTDIR --output-file app.base

# do not call test-all, in order to not overwrite log files
echo "-- test base --"
$TESTDIR/wxex-test-base

echo "-- test gui --"
$TESTDIR/wxex-test-gui

echo "-- test gui report --"
$TESTDIR/wxex-test-gui-report

echo "-- lcov collecting data --"
lcov --base-directory ~/wxExtension/extension --capture --directory $TESTDIR --output-file app.run
lcov --add-tracefile app.base --add-tracefile app.run --output-file app.total

# remove output for external and test libraries
lcov --remove app.total "/usr*" --output-file app.total
lcov --remove app.total "test/*" --output-file app.total
lcov --remove app.total "sample*" --output-file app.total
lcov --remove app.total "*wxExtension/sync*" --output-file app.total

echo "-- genhtml building report --"
genhtml app.total

rm -f app.base app.run app.total
