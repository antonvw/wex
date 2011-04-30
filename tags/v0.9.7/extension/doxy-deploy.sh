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
# by doing svn stat, you see which files are not yet added,
# so you should add them using svn add html/*, and commit afterwards

cd ~/syncped/trunk/doc
cp -rf ~/wxextension/trunk/extension/html/ .
svn commit -m "updated docs"
svn stat
