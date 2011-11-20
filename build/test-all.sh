#!/bin/sh
################################################################################
# Name:      test-all.sh
# Purpose:   script to run all tests
# Author:    Anton van Wezenbeek
# Copyright: (c) 2011 Anton van Wezenbeek
################################################################################

TESTDIR=./gccgtk2_dll/

ORG=$PWD

echo "-- test base --"
$TESTDIR/wxex-test-base > $ORG/test-base.log

echo "-- test gui --"
$TESTDIR/wxex-test-gui > $ORG/test-gui.log

echo "-- test gui report --"
$TESTDIR/wxex-test-gui-report > $ORG/test-gui-report.log

cat *.log
