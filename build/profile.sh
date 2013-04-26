#!/bin/sh
################################################################################
# Name:      profile.sh
# Purpose:   Profile file (for gprof)
# Author:    Anton van Wezenbeek
# Copyright: (c) 2013 Anton van Wezenbeek
################################################################################

# Run this file in the build folder
# If you did another make before, first do a make clean.

export CPPFLAGS="-g -O0 -pg"
export LDFLAGS="-g -O0 -pg"

echo "-- make profile build --"

make

