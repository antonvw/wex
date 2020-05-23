////////////////////////////////////////////////////////////////////////////////
// Name:      data/menu.cpp
// Purpose:   Implementation of wex::menu_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/managedframe.h>
#include <wex/menu-data.h>
#include <wx/app.h>

wex::menu_data& wex::menu_data::action(const action_t rhs)
{
  m_action = rhs;
  return *this;
}

wex::menu_data& wex::menu_data::art(const wxArtID& rhs)
{
  m_artid = rhs;
  return *this;
}

void wex::menu_data::bind(int id) const
{
  if (id == wxID_ANY)
  {
    return;
  }

  auto* frame = dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());

  if (m_action != nullptr)
  {
    frame->Bind(wxEVT_MENU, m_action, id);
  }

  if (m_ui != nullptr)
  {
    frame->Bind(wxEVT_UPDATE_UI, m_ui, id);
  }
}

wex::menu_data& wex::menu_data::help_text(const std::string& rhs)
{
  m_help_text = rhs;
  return *this;
}

wex::menu_data& wex::menu_data::ui(const ui_t rhs)
{
  m_ui = rhs;
  return *this;
}
