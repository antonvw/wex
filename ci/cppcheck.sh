#!/usr/bin/env sh
# This script is used to run cppcheck
# We are already in the build dir

cppcheck --version

cppcheck --quiet --enable=all \
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
  ../src -i LexAda.cxx -i odbc.cpp -I ../include
