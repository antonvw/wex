#!/bin/sh
################################################################################
# Name:      test-all.sh
# Purpose:   script to run all tests
# Author:    Anton van Wezenbeek
# Copyright: (c) 2015 Anton van Wezenbeek
################################################################################

TESTDIR=extension/test

if [ ! -d ~/.wxex-test-base ]; then
  mkdir ~/.wxex-test-base
  cp ../extension/data/*.xml ~/.wxex-test-base
fi

if [ ! -d ~/.wxex-test-gui ]; then
  mkdir ~/.wxex-test-gui
  cp ../extension/data/*.xml ~/.wxex-test-gui
fi

if [ ! -d ~/.wxex-test-gui-report ]; then
  mkdir ~/.wxex-test-gui-report
  cp ../extension/data/*.xml ~/.wxex-test-gui-report
fi

echo "-- test base --"
$TESTDIR/base/wxex-test-base
RC=$?

echo "-- test gui --"
$TESTDIR/gui/wxex-test-gui
NRC=$?

if [ $RC -eq "0" ]; then
  RC=$NRC
fi

echo "-- test gui report --"
$TESTDIR/gui-report/wxex-test-gui-report
NRC=$?

if [ $RC -eq "0" ]; then
  RC=$NRC
fi

return $RC
