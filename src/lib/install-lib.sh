#!/usr/bin/env bash

root=$(git rev-parse --show-toplevel)

build="${root}/build"
dest="/usr/local/Cellar/wex"

# install include files
cp -rfp "${root}"/src/include/wex "${dest}"/include 
cp -rfp "${root}"/external/wxWidgets/include/wx "${dest}"/include 
cp -p "${root}"/external/pugixml/src/pugiconfig.hpp "${dest}"/include 
cp -p "${root}"/external/pugixml/src/pugixml.hpp "${dest}"/include 

# install setup.h
find "${build}" -name "setup.h" -exec cp -p {} "${dest}"/include/wx \;

# install libraries
find "${build}" -name "*.a" -exec cp -p {} "${dest}"/lib \;
