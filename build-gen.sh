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
  echo "usage: build-gen.sh [-B <dir>] [-d <dir>] [-abcghilopstT]"
  echo "-a       build ASAN leak sanitizer"
  echo "-b       build static libraries, default builds shared libraries"
  echo "-B <dir> boost root build <dir>"
  echo "-c       build coverage mode"
  echo "-d <dir> build <dir>, default uses the 'build' subdir"
  echo "-D <x=y> add a general cmake define"
  echo "-g       build github mode"
  echo "-h       displays usage information and exits"
  echo "-i       build interface bindings for SWIG"
  echo "-l       add locale files"
  echo "-o       build ODBC"
  echo "-p       prepare only, do not run ninja after generating build files"
  echo "-s       build samples binaries as well"
  echo "-t       build tests binaries as well"
  echo "-T       build clang-tidy (to enable run-clang-tidy)"
}

option_asan=
option_boost_build=
# use shared libs for Boost, wxWidgets and wex
option_build="-DwexBUILD_SHARED=ON"
option_cmake=
option_coverage=
option_dir=build
option_github=
option_locale=
option_odbc=
option_prepare=false
option_samples=
option_swig=
option_tests=
option_tidy=

while getopts ":B:d:D:abcghilopstT" opt; do
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

    D)
      option_cmake="${option_cmake} -D$OPTARG"
    ;;

    g)
      option_github="-DwexBUILD_GITHUB=ON"
    ;;

    h)
      usage
      exit 1
    ;;

    i)
      option_swig="-DwexBUILD_BINDINGS=ON"
    ;;

    l)
      option_locale="-DwexENABLE_GETTEXT=ON"
    ;;

    o)
      option_odbc="-DwexENABLE_ODBC=ON"
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

    T)
      option_tidy="-DwexBUILD_TIDY=ON"
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
  export CC=${LLVM_DIR}/llvm/bin/clang
  export CXX=${LLVM_DIR}/llvm/bin/clang
fi

mkdir -p "${option_dir}"

cmake -B "${option_dir}" -G Ninja \
  ${option_asan} \
  ${option_boost_build} \
  ${option_build} \
  ${option_cmake} \
  ${option_coverage} \
  ${option_github} \
  ${option_locale} \
  ${option_odbc} \
  ${option_samples} \
  ${option_swig} \
  ${option_tests} \
  ${option_tidy}

if [[ "${option_prepare}" == "false" ]]; then
  cd "${option_dir}" && ninja
fi
