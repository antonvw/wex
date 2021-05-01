#!/usr/bin/env sh

../ci/codespell.sh

# no tests, to reduce time
cmake -DCMAKE_CXX_COMPILER="${CXX}" -DwexBUILD_SHARED=ON ..
make -j 4
