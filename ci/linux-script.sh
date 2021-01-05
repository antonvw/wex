#!/usr/bin/env sh

cmake -DCMAKE_CXX_COMPILER="${CXX}" -DwexBUILD_TESTS=ON ..
make -j 4
cmake ..
sudo make install
test/core/wex-test-core
test/ui/wex-test-ui -tce=wex::debug,wex::file_history
test/report/wex-test-report

cd ..
git clone --branch c++17 https://gitlab.kitware.com/antonvw/syncped.git
cd syncped || exit
mkdir build && cd build || exit
cmake .. && make -j 2
