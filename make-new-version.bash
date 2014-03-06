################################################################################
# Name:      make-new-version.bash
# Purpose:   Update version numbers
# Author:    Anton van Wezenbeek
# Copyright: (c) 2014 Anton van Wezenbeek
################################################################################

if [ $# -ne 2 ]
then
  echo "Usage: `basename $0` old-version-no new-version-no"
  exit 1
fi

find -name "Doxyfile" -exec sed -i s/$1/$2/g '{}' \;
find -name "*.bat" -exec sed -i s/$1/$2/g '{}' \;
find -name "*.h" -exec sed -i s/$1/$2/g '{}' \;
find -name "*.htm" -exec sed -i s/$1/$2/g '{}' \;
find -name "*.sh" -exec sed -i s/$1/$2/g '{}' \;
find -name "*.md" -exec sed -i s/$1/$2/g '{}' \;
