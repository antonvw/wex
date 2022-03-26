#!/usr/bin/env sh

# shellcheck source=../ci/use-clang.sh
. ../ci/use-clang.sh

cmake -DCMAKE_CXX_COMPILER="${CXX}" -DwexBUILD_SAMPLES=ON -DwexBUILD_TESTS=ON -DwexBUILD_SHARED=ON ..

# only make wex targets, to reduce time
#make -j 4 wex-core wex-factory wex-data wex-common wex-ui wex-vi wex-stc wex-del
