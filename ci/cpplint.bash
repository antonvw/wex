#!/usr/bin/env sh
# This script is used to run cpplint manually from root repo

/usr/local/bin/cpplint --quiet --exclude=src/lexers --filter=\
-build,\
-readability/casting,\
-readability/fn_size,\
-readability/multiline_string,\
-runtime/explicit,\
-runtime/indentation_namespace,\
-runtime/int,\
-runtime/references,\
-whitespace\
 "$@"
