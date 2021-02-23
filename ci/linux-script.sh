#!/usr/bin/env sh
# This script is used to build wex on travis using linux
# We are already in the build dir

# build
cmake -DCMAKE_CXX_COMPILER="${CXX}" -DwexBUILD_TESTS=ON -DwexBUILD_SAMPLES=ON ..
make -j 4

# install
cmake ..
sudo make install

# cppcheck
cppcheck --quiet --enable=all \
  --suppress=constStatement \
  --suppress=cppcheckError \
  --suppress=cstyleCast \
  --suppress=noCopyConstructor \
  --suppress=noExplicitConstructor \
  --suppress=noOperatorEq \
  --suppress=ignoredReturnValue \
  --suppress=unusedScopedObject \
  --suppress=unusedFunction\
  ../src -i LexAda.cxx -I ../include 
#  2> err.txt

# test
test/core/wex-test-core
test/ui/wex-test-ui -tce=wex::debug,wex::file_history
test/report/wex-test-report

# build syncped
cd ..
git clone https://gitlab.kitware.com/antonvw/syncped.git
cd syncped || exit
mkdir build && cd build || exit
cmake .. && make -j 2
