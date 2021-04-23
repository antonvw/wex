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
./cppcheck.sh

# test
ctest -V

# build syncped
cd ..
git clone https://gitlab.kitware.com/antonvw/syncped.git
cd syncped || exit
mkdir build && cd build || exit
cmake .. && make -j 2
