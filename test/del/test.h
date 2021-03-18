////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/del/frame.h>
#include <wex/stc.h>

#include "../test.h"

/// Returns name of pane.
const std::string add_pane(wex::frame* frame, wxWindow* pane);

/// Returns the frame.
wex::del::frame* del_frame();

/// Returns the project.
const std::string get_project();

/// Returns an stc.
wex::stc* get_stc();
