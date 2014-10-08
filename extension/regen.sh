#!/bin/bash
# Copied from wxWidgets

# remember current folder and then cd to the docs/doxygen one
me=$(basename $0)
path=${0%%/$me}        # path from which the script has been launched
current=$(pwd)
cd $path
export RELEASE=`wx-config --version`
export WXWIDGETS=`pwd`

# now run doxygen
doxygen

# return to the original folder from which this script was launched
cd $current
