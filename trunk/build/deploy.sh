# Name:      deploy.sh
# Purpose:   Deploy file (for syncped)
# Author:    Anton van Wezenbeek
# Created:   2010-11-28
# RCS-ID:    $Id$
# Copyright: (c) 2010 Anton van Wezenbeek

tar cf syncped.tar \
  syncped \
  /home/anton/wxextension/trunk/extension/data/lexers.xml \
  /home/anton/wxextension/trunk/extension/data/vcs.xml \
  /home/anton/wxextension/trunk/extension/locale/wxextension-nl.mo \
  /home/anton/wxextension/trunk/syncped/locale/syncped-nl.mo
 
gzip syncped.tar
