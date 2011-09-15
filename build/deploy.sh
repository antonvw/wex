#!/bin/sh
################################################################################
# Name:      deploy.sh
# Purpose:   Deploy file (for syncped)
# Author:    Anton van Wezenbeek
# Copyright: (c) 2011 Anton van Wezenbeek
################################################################################

# Run this file in the build folder

mkdir syncped
mkdir syncped/nl-NL

# Copy application.
cp gccgtk2_dll/syncped syncped

# Copy the libs.
cp ~/wxWidgets-2.9.2/buildgtk/lib/libwx*2.9*so*0 syncped

# Copy data.
cp ../extension/data/lexers.xml syncped
cp ../extension/data/vcs.xml syncped

# Copy locale files.
cp ~/wxWidgets-2.9.2/locale/nl.mo syncped/nl-NL/
cp ../locale/wxextension-nl.mo syncped/nl-NL/
cp ../locale/wxstd-xxx-nl.mo syncped/nl-NL/
cp ../locale/syncped-nl.mo syncped/nl-NL/
 
strip syncped/syncped
tar cf syncped.tar syncped
gzip syncped.tar

mv syncped.tar.gz ~/syncped/bin

rm -rf syncped
