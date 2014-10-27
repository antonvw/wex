#!/bin/sh
################################################################################
# Name:      test-all.sh
# Purpose:   script to run all tests
# Author:    Anton van Wezenbeek
# Copyright: (c) 2014 Anton van Wezenbeek
################################################################################

TOOLKIT=`wx-config --query-toolkit`
TESTDIR=./gcc$TOOLKIT\_dll/

if [ ! -d ~/.wxex-test ]; then
  mkdir ~/.wxex-test
  cp ../extension/data/*.xml ~/.wxex-test
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
$TESTDIR/wxex-test-base
RC=$?

echo "-- test gui --"
$TESTDIR/wxex-test-gui
NRC=$?

if [ $RC -eq "0" ]; then
  RC=$NRC
fi

# Skip this test for Travis, it hangs the gui?
if [ ! $TRAVIS ]; then
  echo "-- test gui report --"
  $TESTDIR/wxex-test-gui-report
  NRC=$?
  
  if [ $RC -eq "0" ]; then
    RC=$NRC
  fi
fi

return $RC
