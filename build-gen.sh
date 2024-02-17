#!/bin/bash
# shellcheck disable=SC2086
################################################################################
# Name:      build-gen.sh
# Purpose:   Generates (ninja) build files for osx or linux
#            for building wex itself, or for building apps using it
#            Just run from repo root
# Author:    Anton van Wezenbeek
# Copyright: (c) 2024 Anton van Wezenbeek
################################################################################

usage()
{
  echo "usage: build-gen.sh [-B <dir>] [-d <dir>] [-abcghlopst]"
  echo "-a       build ASAN leak sanitizer"
  echo "-B <dir> boost root build <dir>"
  echo "-b       build static libraries, default builds shared libraries"
  echo "-c       build coverage mode"
  echo "-d <dir> build <dir>, default uses the 'build' subdir"
  echo "-g       build github mode"
  echo "-h       displays usage information and exits"
  echo "-l       add locale files"
  echo "-o       build ODBC"
  echo "-p       prepare only, do not run ninja after generating build files"
  echo "-s       build samples binaries as well"
  echo "-t       build tests binaries as well"
}

option_asan=
option_boost_build=
# use shared libs for Boost, wxWidgets and wex
option_build="-DwexBUILD_SHARED=ON"
option_coverage=
option_dir=build
option_github=
option_locale=
option_odbc=
option_prepare=false
option_samples=
option_tests=

while getopts ":B:d:abcghlopst" opt; do
  case $opt in
    a)
      option_asan="-DwexENABLE_ASAN=ON"
    ;;

    b)
      option_build=
    ;;

    B)
      option_boost_build="-DBoost_INCLUDE_DIR=${OPTARG}/include -DBoost_LIBRARY_DIRS=${OPTARG}/lib"
    ;;

    c)
      option_coverage="-DCMAKE_BUILD_TYPE=Coverage -DwexGCOV=gcov-13"
    ;;

    d)
      option_dir="$OPTARG"
    ;;

    g)
      option_github="-DwexBUILD_GITHUB=ON"
    ;;

    h)
      usage
      exit 1
    ;;

    l)
      option_locale="-DwexENABLE_GETTEXT=ON"
    ;;

    o)
      option_odbc="-wexENABLE_ODBC=ON"
    ;;

    p)
      option_prepare=true
    ;;

    s)
      option_samples="-DwexBUILD_SAMPLES=ON"
    ;;

    t)
      option_tests="-DwexBUILD_TESTS=ON"
    ;;

    :)
      echo "option -${OPTARG} requires an argument"
      exit 1
    ;;

    \?)
      echo "illegal option -$OPTARG"
      exit 1
    ;;
  esac
done

uname=$(uname)

if [[ $uname == "Darwin" ]]; then
  # cmake find_package llvm otherwise gives error
  export LLVM_DIR=/usr/local/Cellar/homebrew/opt
fi

mkdir -p "${option_dir}"

cmake -B "${option_dir}" -G Ninja \
  ${option_asan} \
  ${option_boost_build} \
  ${option_build} \
  ${option_coverage} \
  ${option_github} \
  ${option_locale} \
  ${option_odbc} \
  ${option_samples} \
  ${option_tests}

if [[ "${option_prepare}" == "false" ]]; then
  cd "${option_dir}" && ninja
fi
