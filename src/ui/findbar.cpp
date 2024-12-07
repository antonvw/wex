////////////////////////////////////////////////////////////////////////////////
// Name:      findbar.cpp
// Purpose:   Implementation of wex::find_bar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/find.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frd.h>
#include <wex/ui/grid.h>
#include <wex/ui/listview.h>

#include "findbar.h"

wex::find_bar::find_bar(wex::frame* frame, const data::window& data)
  : ex_commandline(
      frame,
      "@" + find_replace_data::get()->get_find_string(),
      data)
{
  control()->Bind(
    wxEVT_SET_FOCUS,
    [=, this](wxFocusEvent& event)
    {
      if (control() != frame->get_stc())
      {
        set_stc(dynamic_cast<wex::syntax::stc*>(frame->get_stc()));
      }

      event.Skip();
    });
}

bool wex::find_bar::find(bool user_input)
{
  if (auto* grid = dynamic_cast<wex::grid*>(get_frame()->get_grid());
      grid != nullptr)
  {
    const data::find f(get_text(), find_replace_data::get()->search_down());
    return grid->find_next(f);
  }
  else if (auto* lv = get_frame()->get_listview(); lv != nullptr)
  {
    return lv->find_next(get_text(), find_replace_data::get()->search_down());
  }
  else
  {
    return ex_commandline::find(user_input);
  }

  return false;
}

bool wex::find_bar::find_on_enter()
{
  find(false);
  return true;
}
