################################################################################
# Name:      make-new-version.bash
# Purpose:   Update version numbers
# Author:    Anton van Wezenbeek
# Created:   2010-12-11
# RCS-ID:    $Id$
# Copyright: (c) 2010 Anton van Wezenbeek
################################################################################

if [ $# -ne 2 ]
then
  echo "Usage: `basename $0` old-version-no new-version-no"
  exit 1
fi

find -name "Doxyfile" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.cpp" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.h" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.htm" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.po" -exec sed -i s/$1/$2/ '{}' \;
