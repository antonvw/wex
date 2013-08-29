#!/bin/bash
################################################################################
# Name:      deploy.sh
# Purpose:   Deploy file (for syncped)
# Author:    Anton van Wezenbeek
# Copyright: (c) 2013 Anton van Wezenbeek
################################################################################

# Run this file in the build folder

TOOLKIT=`wx-config --query-toolkit`

mkdir syncped
mkdir syncped/fr_FR
mkdir syncped/nl_NL

# Copy application.
cp gcc$TOOLKIT\_dll/syncped syncped

# Copy the libs.
cp ~/wxWidgets-3.0/buildgtk/lib/libwx*3.0*so*0 syncped

# Copy xml and templates data.
cp ../extension/data/*.txt syncped
cp ../extension/data/*.xml syncped

# Copy locale files.
msgfmt ~/wxWidgets-3.0/locale/fr.po -o syncped/fr_FR/fr.mo
msgfmt ~/wxWidgets-3.0/locale/nl.po -o syncped/nl_NL/nl.mo

FILES=( $( /bin/ls ../locale/*fr.po  ) )

for f in $FILES
do
  # name without extension
  name=${f%%.po}
  name=${name##*/}
  msgfmt ../locale/$name.po -o syncped/fr_FR/$name.mo
done

FILES=( $( /bin/ls ../locale/*nl.po  ) )

for f in $FILES
do
  # name without extension
  name=${f%%.po}
  name=${name##*/}
  msgfmt ../locale/$name.po -o syncped/nl_NL/$name.mo
done

strip syncped/syncped
tar cf syncped.tar syncped
gzip syncped.tar

mv syncped.tar.gz ~/syncped/bin

rm -rf syncped
