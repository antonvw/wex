################################################################################
# Name:      doxy-deploy.sh
# Purpose:   deploy html pages
# Author:    Anton van Wezenbeek
# Created:   2011-02-07
# RCS-ID:    $Id$
# Copyright: (c) 2011 Anton van Wezenbeek
################################################################################

# this script only works if there are no new files since previous commit
# other they have to be added first using svn add

cd ~/syncped/trunk/doc
cp -rf ~/wxextension/trunk/extension/html/ .
svn commit -m "updated docs"
