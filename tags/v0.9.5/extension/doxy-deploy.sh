################################################################################
# Name:      doxy-deploy.sh
# Purpose:   deploy html pages
# Author:    Anton van Wezenbeek
# Created:   2011-02-07
# RCS-ID:    $Id$
# Copyright: (c) 2011 Anton van Wezenbeek
################################################################################

cd ~/syncped/trunk/doc
cp -rf ~/wxextension/trunk/extension/html/ .
svn commit -m "updated docs"
