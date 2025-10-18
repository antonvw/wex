////////////////////////////////////////////////////////////////////////////////
// Name:      wex.i 
// Purpose:   SWIG interface file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

%include <std_filesystem.i>
%include <std_pair.i>
%include <std_sstream.i>
%include <std_string.i>
%include <std_vector.i>

// std::stringstream deletes copy constructor needed by wrap

%module wex
%{
#include <wex/common/wex.h>
#include <wex/data/wex.h>
#include <wex/core/wex.h>
#include <wex/factory/wex.h>
%}

%include "../include/wex/core/app.h"
%include "../include/wex/core/chrono.h"
%include "../include/wex/core/config.h"
%include "../include/wex/core/core.h"
%include "../include/wex/core/cmdline-data.h"
%include "../include/wex/core/file-status.h"
%include "../include/wex/core/function-repeat.h"
%include "../include/wex/core/interruptible.h"
%include "../include/wex/core/regex.h"
%include "../include/wex/core/regex-part.h"
%include "../include/wex/core/temp-filename.h"
%include "../include/wex/core/tokenize.h"
%include "../include/wex/core/type-to-value.h"
%include "../include/wex/core/types.h"
%include "../include/wex/core/version.h"

%include "../include/wex/factory/control.h"
%include "../include/wex/factory/defs.h"
%include "../include/wex/factory/frd.h"
%include "../include/wex/factory/process-data.h"
%include "../include/wex/factory/sort.h"
%include "../include/wex/factory/text-window.h"
%include "../include/wex/factory/util.h"

%include "../include/wex/common/statistics.h"
%include "../include/wex/common/tool.h"
