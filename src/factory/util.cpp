////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wex factory utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/frame.h>
#include <wex/factory/util.h>
#include <wx/app.h>

void wex::bind_set_focus(wxWindow* win)
{
  if (auto* frame = dynamic_cast<factory::frame*>(wxTheApp->GetTopWindow());
      frame != nullptr)
  {
    win->Bind(
      wxEVT_SET_FOCUS,
      [frame, win](wxFocusEvent& event)
      {
        frame->set_find_focus(win);
        event.Skip();
      });
  }
}
