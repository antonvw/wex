////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Declaration of factory util functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

class wxWindow;

namespace wex
{
/// Binds to focus.
void bind_set_focus(wxWindow* window);
}; // namespace wex
