#!/bin/bash

function find_sed()
{
  if ! gsed --h > /dev/null;
  then
    export _sed=sed
  else
    export _sed=gsed
  fi
}

function substitute 
{
  ${_sed} -e "s/SOME.*/wex localization file $1/" -i "$1"
  ${_sed} -e "s/YEAR/2017-2021/" -i "$1" 
  ${_sed} -e "s/PACKAGE/wex/" -i "$1" 
  ${_sed} -e "s/VERSION/22.04/" -i "$1" 
  ${_sed} -e "/FIRST AUTHOR.*/d" -i "$1" 
  ${_sed} -e "s/charset=CHARSET/charset=UTF-8/" -i "$1" 
}

find_sed

# file locations
locs="../src/common/*.cpp ../src/core/*.cpp ../src/data/*.cpp ../src/del/*.cpp ../src/factory/*.cpp ../src/stc/*.cpp ../src/ui/*.cpp ../src/vi/*.cpp ../include/wex/*.h ../include/wex/data/*.h ../include/wex/del/*.h ../include/wex/factory/*.h"

# create pot file
xgettext -F -k_ -o wex.pot --no-location --copyright-holder="A.M. van Wezenbeek" "$locs"

# merge (-j join) all po files
for f in *.po; do
  xgettext -F -j -k_ -o "$f" --no-location --copyright-holder="A.M. van Wezenbeek" "$locs"
  substitute "$f"
done  
