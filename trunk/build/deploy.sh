# Name:      deploy.sh
# Purpose:   Deploy file (for syncped)
# Author:    Anton van Wezenbeek
# Created:   2010-11-28
# RCS-ID:    $Id$
# Copyright: (c) 2010 Anton van Wezenbeek

# Run this file in the build folder

tar cf syncped.tar \
  gccgtk2_dll/syncped \
  ../extension/data/lexers.xml \
  ../extension/data/vcs.xml \
  ~/wxWidgets-2.9.1/locale/nl.mo \
  ../extension/locale/wxextension-nl.mo \
  ../extension/locale/wxstd-xxx-nl.mo \
  ../syncped/locale/syncped-nl.mo
 
gzip syncped.tar

cp syncped.tar.gz ~/syncped/bin

cd syncped/bin

svn commit -m "deployed syncped"
