////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/stc/stc.h>
#include <wex/ui/frame.h>

#include "../test.h"

/// Returns the frame.
wex::frame* frame();

/// Returns an stc.
wex::stc* get_stc();
