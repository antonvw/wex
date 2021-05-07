#!/usr/bin/env sh
# This script is used to run cpplint manually from root repo

/usr/local/Cellar/cpplint/1.5.4/bin/cpplint --quiet --exclude=src/lexers --filter=\
-build,\
-readability/fn_size,\
-readability/multiline_string,\
-runtime/explicit,\
-runtime/indentation_namespace,\
-runtime/int,\
-runtime/references,\
-whitespace\
 "$*"
