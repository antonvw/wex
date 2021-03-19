////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wex/frame.h>
#include <wex/stc.h>

#include "../test.h"

/// Adds managed pane.
/// Returns name of pane.
const std::string add_pane(wex::frame* frame, wxWindow* pane);

/// Returns the frame.
wex::frame* frame();

/// Returns an stc.
wex::stc* get_stc();
