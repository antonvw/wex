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

  const bool wrapscan(config(_("stc.Wrap scan")).get(true));

  wex::data::find f(
    this,
    forward || (find_flags == -1 && find_replace_data::get()->search_down()));
  f.flags(find_flags);

  if (m_margin_text_click >= 0)
  {
    if (int line; f.find_margin(text, line))
    {
      data::stc(data::control().line(line + 1), this).inject();
      log::verbose(get_filename().fullname())
        << "found margin text:" << text << "on line:" << line + 1;
      return true;
    }

    return false;
  }

  SetTargetRange(f.start_pos(), f.end_pos());
  set_search_flags(find_flags);

  if (SearchInTarget(text) == -1)
  {
    frame::statustext(
      get_find_result(text, forward, f.recursive()),
      std::string());

    bool found = false;

    if (!f.recursive() && wrapscan)
    {
      f.recursive(true);
      found = find_next(text, find_flags, forward);
      f.recursive(false);

      if (!found)
      {
        log::verbose(get_filename().fullname())
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

    if (m_vi.mode().normal() || m_vi.mode().insert())
    {
      SetSelection(GetTargetStart(), GetTargetEnd());
    }
    else if (m_vi.mode().visual())
    {
      if (forward)
        m_vi.visual_extend(GetSelectionStart(), GetTargetEnd());
      else
        m_vi.visual_extend(GetTargetStart(), GetSelectionEnd());
    }

    EnsureVisible(LineFromPosition(GetTargetStart()));
    EnsureCaretVisible();

    log::verbose(get_filename().fullname()) << "found text:" << text;

    return true;
  }
}
