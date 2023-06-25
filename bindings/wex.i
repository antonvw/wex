////////////////////////////////////////////////////////////////////////////////
// Name:      wex.i 
// Purpose:   SWIG interface file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

%include <std_pair.i>
%include <std_string.i>
%include <std_vector.i>

%module wex
%{
#include <boost/regex.hpp>

#include <wex/core/chrono.h>
#include <wex/core/interruptible.h>
#include <wex/core/regex-part.h>
%}

// currently the auto keyword gives mostly a syntax error
// and std::stringstream deletes copy constructor needed by wrap

%include "../include/wex/core/chrono.h"
%include "../include/wex/core/interruptible.h"
%include "../include/wex/core/regex-part.h"
