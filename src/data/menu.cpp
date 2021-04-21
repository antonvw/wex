////////////////////////////////////////////////////////////////////////////////
// Name:      data/menu.cpp
// Purpose:   Implementation of wex::data::menu
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/menu.h>
#include <wx/app.h>
#include <wx/frame.h>

wex::data::menu& wex::data::menu::action(const action_t rhs)
{
  m_action = rhs;
  return *this;
}

wex::data::menu& wex::data::menu::art(const wxArtID& rhs)
{
  m_artid = rhs;
  return *this;
}

void wex::data::menu::bind(int id) const
{
  if (id == wxID_ANY)
  {
    return;
  }

  auto* frame = dynamic_cast<wxFrame*>(wxTheApp->GetTopWindow());

  if (m_action != nullptr)
  {
    frame->Bind(wxEVT_MENU, m_action, id);
  }

  if (m_ui != nullptr)
  {
    frame->Bind(wxEVT_UPDATE_UI, m_ui, id);
  }
}

wex::data::menu& wex::data::menu::help_text(const std::string& rhs)
{
  m_help_text = rhs;
  return *this;
}

wex::data::menu& wex::data::menu::ui(const ui_t rhs)
{
  m_ui = rhs;
  return *this;
}
