////////////////////////////////////////////////////////////////////////////////
// Name:      bind.cpp
// Purpose:   Implementation of class wex::bind
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/defs.h>
#include <wex/ui/bind.h>
#include <wx/fdrepdlg.h>

wex::bind::bind(wxEvtHandler* evt)
  : m_handler(evt)
{
}

void wex::bind::command(
  std::vector<std::pair<std::function<void(wxCommandEvent&)>, int>> v)
{
  for (const auto& it : v)
  {
    switch (it.second)
    {
      case ID_ALL_CLOSE:
        m_handler->Bind(wxEVT_MENU, it.first, it.second, ID_ALL_SAVE);
        break;

      case ID_EDIT_DEBUG_FIRST:
        m_handler->Bind(wxEVT_MENU, it.first, it.second, ID_EDIT_DEBUG_LAST);
        break;

      case ID_EDIT_VCS_LOWEST:
        m_handler->Bind(wxEVT_MENU, it.first, it.second, ID_EDIT_VCS_HIGHEST);
        break;

      case ID_FIND_FIRST:
        m_handler->Bind(wxEVT_MENU, it.first, it.second, ID_FIND_LAST);
        break;

      case ID_TOOL_LOWEST:
        m_handler->Bind(wxEVT_MENU, it.first, it.second, ID_TOOL_HIGHEST);
        break;

      default:
        m_handler->Bind(wxEVT_MENU, it.first, it.second);
    }
  }
}

void wex::bind::command(
  std::vector<std::tuple<std::function<void(wxCommandEvent&)>, int, int>> v)
{
  for (const auto& it : v)
  {
    m_handler
      ->Bind(wxEVT_MENU, std::get<0>(it), std::get<1>(it), std::get<2>(it));
  }
}

void wex::bind::frd(
  wxFindReplaceData*                            frd,
  std::function<void(const std::string&, bool)> f)
{
  m_handler->Bind(
    wxEVT_FIND,
    [=, this](wxFindDialogEvent& event)
    {
      f(frd->GetFindString(), (frd->GetFlags() & wxFR_DOWN) > 0);
    });

  m_handler->Bind(
    wxEVT_FIND_NEXT,
    [=, this](wxFindDialogEvent& event)
    {
      f(frd->GetFindString(), (frd->GetFlags() & wxFR_DOWN) > 0);
    });
}

void wex::bind::ui(
  std::vector<std::pair<std::function<void(wxUpdateUIEvent&)>, int>> v)
{
  for (const auto& it : v)
  {
    m_handler->Bind(wxEVT_UPDATE_UI, it.first, it.second);
  }
}
