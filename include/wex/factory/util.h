////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Declaration of factory util functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

class wxEvtHandler;

namespace wex
{
/// Binds to focus.
void bind_set_focus(wxEvtHandler* handler);
}; // namespace wex
