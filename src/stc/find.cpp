////////////////////////////////////////////////////////////////////////////////
// Name:      stc/find.cpp
// Purpose:   Implementation of class wex::stc find methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/data/find.h>
#include <wex/ex/ex-stream.h>
#include <wex/stc/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>

namespace wex
{
bool find_margin(data::find& f, factory::frame* frame)
{
  if (int line = 0; f.find_margin(line))
  {
    data::stc(
      data::control().line(line + 1),
      dynamic_cast<syntax::stc*>(f.stc()))
      .inject();
    log::trace(f.stc()->path().filename())
      << "found margin text:" << f.text() << "on line:" << line + 1;
    return true;
  }

  return false;
}

bool find_other(const vi& vi, data::find& f, factory::frame* frame)
{
  f.stc()->SetTargetRange(f.start_pos(), f.end_pos());

  std::string stext(f.text());

  // match word related to regex ECMAScript
  if (
    f.flags() != -1 && (f.flags() & wxSTC_FIND_CXX11REGEX) &&
    (f.flags() & wxSTC_FIND_WHOLEWORD))
  {
    stext = "\\b" + f.text() + "\\b";
  }

  const bool wrapscan(config(_("stc.Wrap scan")).get(true));

  if (f.stc()->SearchInTarget(stext) == -1)
  {
    f.statustext();

    bool found = false;

    if (!f.recursive() && wrapscan)
    {
      f.recursive(true);
      found = f.stc()->find(f.text(), f.flags(), f.is_forward());
      f.recursive(false);

      if (!found)
      {
        log::trace(f.stc()->path().filename())
          << "text:" << f.text() << "not found";
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

    if (vi.mode().is_command() || vi.mode().is_insert())
    {
      f.stc()->SetSelection(f.stc()->GetTargetStart(), f.stc()->GetTargetEnd());
    }
    else if (f.is_forward())
    {
      vi.visual_extend(f.stc()->GetSelectionStart(), f.stc()->GetTargetEnd());
    }
    else
    {
      vi.visual_extend(f.stc()->GetTargetStart(), f.stc()->GetSelectionEnd());
    }

    f.stc()->EnsureVisible(
      f.stc()->LineFromPosition(f.stc()->GetTargetStart()));
    f.stc()->EnsureCaretVisible();

    log::trace(f.stc()->path().filename()) << "found text:" << f.text();

    return true;
  }
}
} // namespace wex

bool wex::stc::find(const std::string& text, int find_flags, bool forward)
{
  if (text.empty())
  {
    return false;
  }

  wex::data::find f(
    this,
    text,
    forward || (find_flags == -1 && find_replace_data::get()->search_down()));

  f.flags(find_flags);

  switch (m_vi->visual())
  {
    case ex::EX:
      return m_file.ex_stream()->find_data(f);

    case ex::VISUAL:
      if (m_margin_text_click >= 0)
      {
        return find_margin(f, m_frame);
      }
      else
      {
        set_search_flags(find_flags);
        return m_vi->is_active() && find_other(*m_vi, f, m_frame);
      }
      break;

    default:
      return false;
  }
}

bool wex::stc::find_next(bool stc_find_string)
{
  return find(
    stc_find_string ? get_find_string() :
                      find_replace_data::get()->get_find_string(),
    -1);
}
