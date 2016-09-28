#!/bin/bash

function substitute 
{
  sed -e "s/SOME.*/wxExtension localization file $1/" -i $1.org $1
  sed -e "s/YEAR/2017/" -i $1.org $1
  sed -e "s/PACKAGE/wxExtension/" -i $1.org $1
  sed -e "s/VERSION/17.04/" -i $1.org $1
  sed -e "/FIRST AUTHOR.*/d" -i $1.org $1 
  sed -e "s/charset=CHARSET/charset=UTF-8/" -i $1.org $1 
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
