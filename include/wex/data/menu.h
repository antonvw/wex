////////////////////////////////////////////////////////////////////////////////
// Name:      data/menu.h
// Purpose:   Declaration of wex::data::menu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h> // for wxArtID
#include <wx/event.h>

namespace wex::data
{
/// Offers user data to be used by item.
class menu
{
public:
  /// Callback for menu action.
  typedef std::function<void(wxCommandEvent&)> action_t;

  /// Callback for menu update action.
  typedef std::function<void(wxUpdateUIEvent&)> ui_t;

  /// Sets action.
  menu& action(const action_t& rhs);

  /// Returns art.
  auto& art() const { return m_artid; }

  /// Sets art.
  menu& art(const wxArtID& rhs);

  /// Binds action or ui to frame.
  void bind(int id) const;

  /// Returns help text.
  auto& help_text() const { return m_help_text; }

  /// Sets help text.
  menu& help_text(const std::string& rhs);

  /// Sets ui.
  menu& ui(const ui_t& rhs);

private:
  std::string m_help_text;
  wxArtID     m_artid;
  action_t    m_action{nullptr};
  ui_t        m_ui{nullptr};
};
} // namespace wex::data
