# Name:      deploy.sh
# Purpose:   Deploy file (for syncped)
# Author:    Anton van Wezenbeek
# Created:   2010-11-28
# RCS-ID:    $Id$
# Copyright: (c) 2010 Anton van Wezenbeek

# Run this file in the build folder

mkdir syncped
mkdir syncped/nl-NL

cp gccgtk2_dll/syncped syncped
cp ~/wxWidgets-2.9.1/buildgtk/lib/libwx*2.9*so.1.0.0 syncped
cp ../extension/data/lexers.xml syncped
cp ../extension/data/vcs.xml syncped
cp ~/wxWidgets-2.9.1/locale/nl.mo syncped/nl-NL/nl.mo
cp ../extension/locale/wxextension-nl.mo syncped/nl-NL/wxextension-nl.mo
cp ../extension/locale/wxstd-xxx-nl.mo syncped/nl-NL/wxstd-xxx-nl.mo
cp ../syncped/locale/syncped-nl.mo syncped/nl-NL/syncped-nl.mo
 
tar cf syncped.tar syncped
gzip syncped.tar

mv syncped.tar.gz ~/syncped/trunk/bin

rm -rf syncped

cd ~/syncped/trunk/bin

svn commit -m "deployed syncped"
