################################################################################
# Name:      make-new-version.bash
# Purpose:   Update version numbers
# Author:    Anton van Wezenbeek
# Copyright: (c) 2012 Anton van Wezenbeek
################################################################################

if [ $# -ne 2 ]
then
  echo "Usage: `basename $0` old-version-no new-version-no"
  exit 1
fi

find -name "Doxyfile" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.bat" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.cpp" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.h" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.htm" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.po" -exec sed -i s/$1/$2/ '{}' \;
find -name "*.sh" -exec sed -i s/$1/$2/ '{}' \;
