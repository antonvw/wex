#!/bin/bash
################################################################################
# Name:      deploy.sh
# Purpose:   Deploy file (for syncped)
# Author:    Anton van Wezenbeek
# Copyright: (c) 2015 Anton van Wezenbeek
################################################################################

RELEASE=`wx-config --release`
VERSION=`wx-config --version`

mkdir app
mkdir app/fr_FR
mkdir app/nl_NL

# Copy application.
cp syncped app

# Copy the libs.
cp ~/wxWidgets-$VERSION/buildgtk/lib/libwx*$RELEASE*so*0 app

# Copy xml and templates data.
cp ../../extension/data/*.txt app
cp ../../extension/data/*.xml app
cp ../../extension/data/*.xsl app

# Copy locale files.
msgfmt ~/wxWidgets-$VERSION/locale/fr.po -o app/fr_FR/fr.mo
msgfmt ~/wxWidgets-$VERSION/locale/nl.po -o app/nl_NL/nl.mo

FILES=../../locale/*fr.po

for f in $FILES
do
  # name without extension
  name=${f%%.po}
  name=${name##*/}
  msgfmt ../locale/$name.po -o app/fr_FR/$name.mo
done

FILES=../../locale/*nl.po

for f in $FILES
do
  # name without extension
  name=${f%%.po}
  name=${name##*/}
  msgfmt ../locale/$name.po -o app/nl_NL/$name.mo
done

strip app/syncped
tar -zcf syncped-v$VERSION.tar.gz app

rm -rf app
