////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

bool wex::test::stc_app::OnInit()
{
  if (!test::app::OnInit())
  {
    return false;
  }

  m_frame = new wex::frame();

  SetTopWindow(m_frame);

  m_stc = new wex::stc();
  m_frame->Show();

  m_frame->pane_add(m_stc);

  return true;
}

wex::frame* frame()
{
  return wex::test::stc_app::frame();
}

wex::stc* get_stc()
{
  return wex::test::stc_app::get_stc();
}

void event(wxWindow* win, char id)
{
  wxKeyEvent evt(wxEVT_CHAR);
  evt.m_uniChar = id;
  wxPostEvent(win, evt);
}

void event(wxWindow* win, const std::string& ids)
{
  for (const auto id : ids)
  {
    event(win, id);
  }

  wxYield();
}

void event(wxWindow* win, int id, wxEventType type)
{
  wxKeyEvent ke(type);
  ke.m_keyCode = id;
  wxPostEvent(win, ke);

  wxYield();
}
