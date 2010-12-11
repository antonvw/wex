################################################################################
# Name:      make-new-version.bash
# Purpose:   Update version numbers
# Author:    Anton van Wezenbeek
# Created:   2010-12-11
# RCS-ID:    $Id$
# Copyright: (c) 2010 Anton van Wezenbeek
################################################################################

find -name *.po Doxyfile *.h *.cpp -exec sed s/$1/$2/ '{}' \;
