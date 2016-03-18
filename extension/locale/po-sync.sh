#!/bin/bash

function substitute 
{
  sed -i "s/SOME.*/wxExtension localization file $1/" $1
  sed -i "s/YEAR/2016/" $1
  sed -i "s/PACKAGE/wxExtension/" $1
  sed -i "s/VERSION/3.1/" $1
  sed -i "/FIRST AUTHOR.*/d" $1 
  sed -i "s/charset=CHARSET/charset=UTF-8/" $1 
}

# file locations
locs="../src/*.cpp ../src/report/*.cpp ../include/wx/extension/*.h ../include/wx/extension/report/*.h"

# create pot file
xgettext -F -k_ -o wxex.pot --copyright-holder="A.M. van Wezenbeek" $locs
substitute wxex.pot

# merge (join) all po files (use --no-location or -F)
for f in *.po; do
  xgettext -F -j -k_ -o $f --copyright-holder="A.M. van Wezenbeek" $locs
  substitute $f
done  
