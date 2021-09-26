////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/del/frame.h>
#include <wex/stc/stc.h>

#include "../test.h"

/// Returns the frame.
wex::del::frame* del_frame();

/// Returns the project.
const wex::path get_project();

/// Returns an stc.
wex::stc* get_stc();
