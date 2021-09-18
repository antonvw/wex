////////////////////////////////////////////////////////////////////////////////
// Name:      bind.h
// Purpose:   Declaration of class wex::bind
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/event.h>

import<vector>;

class wxFindReplaceData;

namespace wex
{
/// Offers a class to bind events to a handler.
class bind
{
public:
  /// Constructor.
  explicit bind(wxEvtHandler* evt);

  /// Binds command events to handler.
  void command(std::vector<std::pair<
                 /// the callback
                 std::function<void(wxCommandEvent&)>,
                 /// the window id, or first for a range of id's (see defs.h)
                 int>>);

  /// Binds find replace data to handler.
  void
  frd(wxFindReplaceData* frd, std::function<void(const std::string&, bool)> f);

  /// Binds update events to handler.
  void ui(std::vector<std::pair<
            /// the callback
            std::function<void(wxUpdateUIEvent&)>,
            /// the window id
            int>>);

private:
  wxEvtHandler* m_handler;
};
}; // namespace wex
