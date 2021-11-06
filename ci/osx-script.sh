#!/usr/bin/env sh

# A normal build, including tests, triggers the max time limit:
# The job exceeded the maximum time limit for jobs, and has been terminated.
  
# no tests, to reduce time
cmake -DCMAKE_CXX_COMPILER="${CXX}" -DwexBUILD_SHARED=ON ..

# only make wex targets, to reduce time
make -j 4 wex-core wex-factory wex-data wex-common wex-ui wex-vi wex-stc wex-del
