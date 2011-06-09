# bash
# File:    test-all.sh
# Purpose: script to run all tests
# RCS-ID:  $Id$

TESTDIR=./gccgtk2_dll/

ORG=$PWD

echo "-- test base --"
$TESTDIR/wxex-test-base > $ORG/test-base.log

echo "-- test app --"
$TESTDIR/wxex-test-app > $ORG/test-app.log

echo "-- test report --"
$TESTDIR/wxex-test-rep > $ORG/test-rep.log

# Commit changes (locally)
git commit -am "results of running test-all"
