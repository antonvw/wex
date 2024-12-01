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
      "/" + find_replace_data::get()->get_find_string(),
      data)
{
  control()->Bind(
    wxEVT_SET_FOCUS,
    [=, this](wxFocusEvent& event)
    {
      if (auto* stc = dynamic_cast<wex::syntax::stc*>(frame->get_stc());
          stc != nullptr)
      {
        stc->position_save();
        set_stc(stc);
        m_stc = stc;
      }
      else
      {
        m_stc = nullptr;
      }
      event.Skip();
    });

  control()->Bind(
    wxEVT_KEY_DOWN,
    [=, this](wxKeyEvent& event)
    {
      if (event.GetKeyCode() == WXK_RETURN)
      {
        event.Skip();

        if (!get_text().empty())
        {
          find_replace_data::get()->set_find_string(get_text());
          find();
        }
      }
      else
      {
        event.Skip();
      }
    });

  control()->Bind(
    wxEVT_STC_CHARADDED,
    [=, this](wxStyledTextEvent& event)
    {
      event.Skip();

      if (m_stc == nullptr)
      {
        find(true, true);
      }
    });
}

bool wex::find_bar::find(bool find_next, bool restore_position)
{
  if (auto* grid = dynamic_cast<wex::grid*>(get_frame()->get_grid());
      grid != nullptr)
  {
    const data::find f(get_text(), find_next);
    return grid->find_next(f);
  }
  else if (auto* lv = get_frame()->get_listview(); lv != nullptr)
  {
    return lv->find_next(get_text(), find_next);
  }
  else if (m_stc != nullptr)
  {
    return m_stc->find(get_text(), -1, find_next);
  }

  return false;
}
