////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wex/factory/stc.h>
#include <wex/frame.h>

#include "../test.h"

/// Adds managed pane.
/// Returns name of pane.
const std::string add_pane(wex::frame* frame, wxWindow* pane);

/// Returns the frame.
wex::frame* frame();

/// Returns the statusbar.
wex::statusbar* get_statusbar();

/// Returns an stc.
wex::factory::stc* get_stc();
