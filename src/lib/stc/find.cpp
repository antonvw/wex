////////////////////////////////////////////////////////////////////////////////
// Name:      stc/find.cpp
// Purpose:   Implementation of class wex::stc find methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/core.h>
#include <wex/find-data.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/stc.h>

namespace wex
{
  bool find_margin(const std::string& text, data::find& f)
  {
    if (int line = 0; f.find_margin(text, line))
    {
      data::stc(data::control().line(line + 1), f.stc()).inject();
      log::verbose(f.stc()->get_filename().fullname())
        << "found margin text:" << text << "on line:" << line + 1;
      return true;
    }

    return false;
  }
  
  bool find_other(const std::string& text, const vi& vi, data::find& f, frame* frame)
  {
    f.stc()->SetTargetRange(f.start_pos(), f.end_pos());

    std::string stext(text);

    // match word related to regex ECMAScript
    if (
      f.flags() != -1 && (f.flags() & wxSTC_FIND_CXX11REGEX) &&
      (f.flags() & wxSTC_FIND_WHOLEWORD))
    {
      stext = "\\b" + text + "\\b";
    }

    const bool wrapscan(config(_("stc.Wrap scan")).get(true));

    if (f.stc()->SearchInTarget(stext) == -1)
    {
      frame->statustext(
        get_find_result(text, f.forward(), f.recursive()),
        std::string());

      bool found = false;

      if (!f.recursive() && wrapscan)
      {
        f.recursive(true);
        found = f.stc()->find_next(text, f.flags(), f.forward());
        f.recursive(false);

        if (!found)
        {
          log::verbose(f.stc()->get_filename().fullname())
            << "text:" << text << "not found";
        }
      }

      return found;
    }
    else
    {
      if (!f.recursive())
      {
        log::status(std::string());
      }

      f.recursive(false);

      if (vi.mode().normal() || vi.mode().insert())
      {
        f.stc()->SetSelection(f.stc()->GetTargetStart(), f.stc()->GetTargetEnd());
      }
      else if (vi.mode().visual())
      {
        if (f.forward())
          vi.visual_extend(f.stc()->GetSelectionStart(), f.stc()->GetTargetEnd());
        else
          vi.visual_extend(f.stc()->GetTargetStart(), f.stc()->GetSelectionEnd());
      }

      f.stc()->EnsureVisible(f.stc()->LineFromPosition(f.stc()->GetTargetStart()));
      f.stc()->EnsureCaretVisible();

      log::verbose(f.stc()->get_filename().fullname()) << "found text:" << text;

      return true;
    }
  }
}

bool wex::stc::find_next(bool stc_find_string)
{
  return find_next(
    stc_find_string ? get_find_string() :
                      find_replace_data::get()->get_find_string(),
    -1);
}

bool wex::stc::find_next(const std::string& text, int find_flags, bool forward)
{
  if (text.empty())
  {
    return false;
  }

  wex::data::find f(
    this,
    forward || (find_flags == -1 && find_replace_data::get()->search_down()));
  f.flags(find_flags);

  if (m_margin_text_click >= 0)
  {
    return find_margin(text, f);
  }
  else
  {
    set_search_flags(find_flags);
    return find_other(text, m_vi, f, m_frame);
  }
}
