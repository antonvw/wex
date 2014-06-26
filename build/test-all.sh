#!/bin/sh
################################################################################
# Name:      test-all.sh
# Purpose:   script to run all tests
# Author:    Anton van Wezenbeek
# Copyright: (c) 2014 Anton van Wezenbeek
################################################################################

TOOLKIT=`wx-config --query-toolkit`
TESTDIR=./gcc$TOOLKIT\_dll/

ORG=$PWD

if [ ! -d ~/.wxex-test ]; then
  mkdir ~/.wxex-test
  cp ../extension/data/*.xml ~/.wxex-test
fi

if [ ! -d ~/.wxex-test-gui ]; then
  mkdir ~/.wxex-test-gui
  cp ../extension/data/*.xml ~/.wxex-test-gui
fi

if [ ! -d ~/.wxex-test-rep ]; then
  mkdir ~/.wxex-test-rep
  cp ../extension/data/*.xml ~/.wxex-test-rep
fi

echo "-- test base --"
$TESTDIR/wxex-test-base > $ORG/test-base.log

echo "-- test gui --"
$TESTDIR/wxex-test-gui > $ORG/test-gui.log

echo "-- test gui report --"
$TESTDIR/wxex-test-gui-report > $ORG/test-gui-report.log

cat *.log
