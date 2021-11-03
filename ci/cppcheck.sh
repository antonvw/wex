#!/usr/bin/env sh
# This script is used to run cppcheck from root repo

cppcheck --version

cppcheck --std=c++20 \
  --quiet --enable=all \
  --suppress=constStatement \
  --suppress=cppcheckError \
  --suppress=cstyleCast \
  --suppress=internalAstError \
  --suppress=noCopyConstructor \
  --suppress=noExplicitConstructor \
  --suppress=noOperatorEq \
  --suppress=ignoredReturnValue \
  --suppress=unknownMacro \
  --suppress=unusedScopedObject \
  --suppress=unusedFunction\
  src -i LexAda.cxx -i odbc.cpp -I include
