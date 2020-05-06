////////////////////////////////////////////////////////////////////////////////
// Name:      menu-data.h
// Purpose:   Declaration of wex::menu_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h> // for wxArtID
#include <wx/event.h>

namespace wex
{
  class menu;

  /// Offers user data to be used by item.
  class menu_data
  {
  public:
    /// Callback for menu action.
    typedef std::function<void(wxCommandEvent&)> action_t;

    /// Callback for menu update action.
    typedef std::function<void(wxUpdateUIEvent&)> ui_t;

    /// Sets action.
    menu_data& action(const action_t rhs);

    /// Returns art.
    auto& art() const { return m_artid; };

    /// Sets art.
    menu_data& art(const wxArtID& rhs);

    /// Binds action or ui to frame.
    void bind(int id) const;

    /// Returns help text.
    auto& help_text() const { return m_help_text; };

    /// Sets help text.
    menu_data& help_text(const std::string& rhs);

    /// Sets ui.
    menu_data& ui(const ui_t rhs);

  private:
    std::string m_help_text;
    wxArtID     m_artid;
    action_t    m_action{nullptr};
    ui_t        m_ui{nullptr};
  };
} // namespace wex
