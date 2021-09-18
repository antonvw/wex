////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/stc.h>
#include <wex/frame.h>

#include "../test.h"

std::vector<std::string> get_builtin_variables();

/// Returns the frame.
wex::frame* frame();

/// Returns an stc.
wex::factory::stc* get_stc();
