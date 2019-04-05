#!/bin/bash

function substitute 
{
  sed -i "s/SOME.*/syncped localization file $1/" $1
  sed -i "s/YEAR/2017/" $1
  sed -i "s/PACKAGE/syncped/" $1
  sed -i "s/VERSION/18.10/" $1
  sed -i "/FIRST AUTHOR.*/d" $1 
  sed -i "s/charset=CHARSET/charset=UTF-8/" $1 
}

# file locations
locs="../*.cpp ../*.h"

# create pot file
xgettext -F -k_ -o syncped.pot --copyright-holder="A.M. van Wezenbeek" $locs

# merge (join) all po files
for f in *.po; do
#  xgettext --no-location -j -k_ -o $f --copyright-holder="A.M. van Wezenbeek" $locs
  xgettext -F -j -k_ -o $f --copyright-holder="A.M. van Wezenbeek" $locs
  substitute $f
done  
