////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wex factory utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/factory/frame.h>
#include <wex/factory/frd.h>
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
  else
  {
    log("no frame available");
  }
}

boost::regex::flag_type
wex::get_regex_flags(const factory::find_replace_data& data)
{
  boost::regex::flag_type flags = boost::regex::ECMAScript;

  if (!data.match_case())
  {
    flags |= boost::regex::icase;
  }

  return flags;
}
