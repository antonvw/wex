#!/bin/sh
################################################################################
# Name:      deploy.sh
# Purpose:   Deploy file (for syncped)
# Author:    Anton van Wezenbeek
# Copyright: (c) 2012 Anton van Wezenbeek
################################################################################

# Run this file in the build folder

mkdir syncped
mkdir syncped/fr-FR
mkdir syncped/nl-NL

# Copy application.
cp gccgtk2_dll/syncped syncped

# Copy the libs.
cp ~/wxWidgets-2.9.4/buildgtk/lib/libwx*2.9*so*4 syncped

# Copy xml and templates data.
cp ../extension/data/*.tpl syncped
cp ../extension/data/*.xml syncped

# Copy locale files.
msgfmt ~/wxWidgets-2.9.4/locale/fr.mo -o syncped/fr-FR/fr.po
msgfmt ~/wxWidgets-2.9.4/locale/nl.mo -o syncped/nl-NL/nl.po

FILES=( $( /bin/ls ../locale/*fr.po  ) )

for f in $FILES
do
  # name without extension
  name=${f%\.*}
  msgfmt ../locale/$name.po -o syncped/fr-FR/$name.mo
done

FILES=( $( /bin/ls ../locale/*nl.po  ) )

for f in $FILES
do
  # name without extension
  name=${f%\.*}
  msgfmt ../locale/$name.po -o syncped/nl-NL/$name.mo
done

strip syncped/syncped
tar cf syncped.tar syncped
gzip syncped.tar

mv syncped.tar.gz ~/syncped/bin

rm -rf syncped
