////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/stc/stc.h>
#include <wex/ui/frame.h>

#include "../test.h"

/// Returns the frame.
wex::frame* frame();

/// Returns an stc.
wex::stc* get_stc();

/// Sends char event to window.
void event(wxWindow* win, char id);

/// Sends char events to window (each char is a separate event).
void event(wxWindow* win, const std::string& ids);

/// Sends key event to window.
void event(wxWindow* win, int id);
