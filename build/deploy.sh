# Name:      deploy.sh
# Purpose:   Deploy file (for syncped)
# Author:    Anton van Wezenbeek
# Created:   2010-11-28
# RCS-ID:    $Id$
# Copyright: (c) 2010 Anton van Wezenbeek

# Run this file in the build folder

mkdir syncped
mkdir syncped/nl-NL

# Copy application.
cp gccgtk2_dll/syncped syncped

# Copy the .1 version.
cp ~/wxWidgets-2.9.2/buildgtk/lib/libwx*2.9*so.1 syncped

# Copy data.
cp ../extension/data/lexers.xml syncped
cp ../extension/data/vcs.xml syncped

# Copy locale files.
cp ~/wxWidgets-2.9.2/locale/nl.mo syncped/nl-NL/
cp ../extension/locale/wxextension-nl.mo syncped/nl-NL/
cp ../extension/locale/wxstd-xxx-nl.mo syncped/nl-NL/
cp ../syncped/locale/syncped-nl.mo syncped/nl-NL/
 
strip syncped/syncped
tar cf syncped.tar syncped
gzip syncped.tar

mv syncped.tar.gz ~/syncped/trunk/bin

rm -rf syncped

cd ~/syncped/trunk/bin

svn commit -m "deployed syncped"
