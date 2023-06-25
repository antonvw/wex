# bindings

This directory will be used if wexBUILD_BINDINGS was specified,
and [swig](https://www.swig.org/) is installed.

This directory contains language bindings for wex.
At this moment python is supported using swig, but stil under
development.

For OSX you need to make a symbolic link '_wex.so' to '_wex.dylib' in
build/bindings, or in the installed python directory.
