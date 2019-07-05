#!/bin/bash

function substitute 
{
  sed -e "s/SOME.*/wex localization file $1/" -i $1 
  sed -e "s/YEAR/2017/" -i $1 
  sed -e "s/PACKAGE/wex/" -i $1 
  sed -e "s/VERSION/19.10/" -i $1 
  sed -e "/FIRST AUTHOR.*/d" -i $1 
  sed -e "s/charset=CHARSET/charset=UTF-8/" -i $1 
}

# file locations
locs="../src/*.cpp ../src/report/*.cpp ../include/wex/*.h ../include/wex/report/*.h"

# create pot file
xgettext -F -k_ -o wex.pot --copyright-holder="A.M. van Wezenbeek" $locs

# merge (join) all po files (use --no-location or -F)
for f in *.po; do
  xgettext -F -j -k_ -o $f --copyright-holder="A.M. van Wezenbeek" $locs
  substitute $f
done  
