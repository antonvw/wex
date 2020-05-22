////////////////////////////////////////////////////////////////////////////////
// Name:      bind.h
// Purpose:   Declaration of class wex::bind
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/event.h>

namespace wex
{
  /// Offers a styled text ctrl with:
  class bind
  {
  public:
    /// Constructor.
    bind(wxEvtHandler* evt);

    /// Binds menu events to handler.
    void
      menus(std::vector<std::pair<std::function<void(wxCommandEvent&)>, int>>);

  private:
    wxEvtHandler* m_handler;
  };
}; // namespace wex
