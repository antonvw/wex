////////////////////////////////////////////////////////////////////////////////
// Name:      bind.cpp
// Purpose:   Implementation of class wex::bind
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/bind.h>
#include <wex/defs.h>

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

      case ID_VCS_LOWEST:
        m_handler->Bind(wxEVT_MENU, it.first, it.second, ID_VCS_HIGHEST);
        break;

      case wxID_SORT_ASCENDING:
        m_handler->Bind(wxEVT_MENU, it.first, it.second, wxID_SORT_ASCENDING);
        break;

      default:
        if (it.second == ID_EDIT_EOL_DOS)
        {
          m_handler->Bind(wxEVT_MENU, it.first, it.second, ID_EDIT_EOL_MAC);
        }
        else
        {
          m_handler->Bind(wxEVT_MENU, it.first, it.second);
        }
    }
  }
}

void wex::bind::ui(
  std::vector<std::pair<std::function<void(wxUpdateUIEvent&)>, int>> v)
{
  for (const auto& it : v)
  {
    m_handler->Bind(wxEVT_UPDATE_UI, it.first, it.second);
  }
}
