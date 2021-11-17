////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

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
