#!/bin/bash
#
# $Id$
# Copied from wxWidgets


# remember current folder and then cd to the docs/doxygen one
me=$(basename $0)
path=${0%%/$me}        # path from which the script has been launched
current=$(pwd)
cd $path
export WXWIDGETS=`pwd`

#
# NOW RUN DOXYGEN
#
# NB: we do this _after_ copying the required files to the output folders
#     otherwise when generating the CHM file with Doxygen, those files are
#     not included!
#
doxygen

# return to the original folder from which this script was launched
cd $current
