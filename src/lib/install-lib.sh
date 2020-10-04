################################################################################
# Name:      install-lib.sh
# Purpose:   Installs wex libs and include files
# Author:    Anton van Wezenbeek
# Copyright: (c) 2020 Anton van Wezenbeek
################################################################################
  
#!/usr/bin/env bash

root=$(git rev-parse --show-toplevel)

build="${root}/build"

# See wex.cmake, locations here should match with wex.cmake.
dest_include="/usr/local/include/wex"
dest_lib="/usr/local/lib/wex"

mkdir -p "${dest_include}"
mkdir -p "${dest_lib}"

# install include files
cp -rfp "${root}"/src/include/wex "${dest_include}"
cp -rfp "${root}"/external/wxWidgets/include/wx "${dest_include}"
cp -p "${root}"/external/pugixml/src/pugiconfig.hpp "${dest_include}"
cp -p "${root}"/external/pugixml/src/pugixml.hpp "${dest_include}"

# install setup.h
find "${build}" -name "setup.h" -exec cp -p {} "${dest_include}"/wx \;

# install libraries
find "${build}" -name "*.a" -exec cp -p {} "${dest_lib}" \;
