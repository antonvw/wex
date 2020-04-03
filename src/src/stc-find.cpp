////////////////////////////////////////////////////////////////////////////////
// Name:      stc-find.cpp
// Purpose:   Implementation of class wex::stc Find methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/util.h>

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

  static bool recursive = false;
  static int  start_pos, end_pos;
  const bool  wrapscan(config(_("stc.Wrap scan")).get(true));

  if (forward || (find_flags == -1 && find_replace_data::get()->search_down()))
  {
    if (recursive)
    {
      start_pos = 0;
      end_pos   = GetCurrentPos();
    }
    else
    {
      start_pos = GetCurrentPos();
      end_pos   = GetTextLength();
    }
  }
  else
  {
    if (recursive)
    {
      start_pos = GetTextLength();
      end_pos   = GetCurrentPos();
      if (GetSelectionStart() != -1)
        end_pos = GetSelectionStart();
    }
    else
    {
      start_pos = GetCurrentPos();
      if (GetSelectionStart() != -1)
        start_pos = GetSelectionStart();
      end_pos = 0;
    }
  }

  if (m_margin_text_click >= 0)
  {
    bool                                            found = false;
    static int                                      line;
    std::match_results<std::string::const_iterator> m;

    if (forward)
    {
      line = LineFromPosition(start_pos) + 1;

      do
      {
        if (const std::string margin(MarginGetText(line));
            ((find_flags & wxSTC_FIND_REGEXP) &&
             std::regex_search(margin, m, std::regex(text))) ||
            margin.find(text) != std::string::npos)
        {
          found = true;
        }
        else
        {
          line++;
        }
      } while (line <= LineFromPosition(end_pos) && !found);

      if (!found && !recursive && wrapscan)
      {
        recursive = true;
        found     = find_next(text, find_flags, forward);
        recursive = false;
      }
    }
    else
    {
      line = LineFromPosition(start_pos) - 1;

      do
      {
        if (const std::string margin(MarginGetText(line));
            ((find_flags & wxSTC_FIND_REGEXP) &&
             std::regex_search(margin, m, std::regex(text))) ||
            margin.find(text) != std::string::npos)
        {
          found = true;
        }
        else
        {
          line--;
        }
      } while (line >= LineFromPosition(end_pos) && !found);

      if (!found && !recursive && wrapscan)
      {
        recursive = true;
        found     = find_next(text, find_flags, forward);
        recursive = false;
      }
    }

    if (found)
    {
      stc_data(control_data().line(line + 1), this).inject();
      log::verbose(get_filename().fullname())
        << "found margin text:" << text << "on line:" << line + 1;
    }

    return found;
  }

  SetTargetRange(start_pos, end_pos);
  set_search_flags(find_flags);

  if (SearchInTarget(text) == -1)
  {
    frame::statustext(get_find_result(text, forward, recursive), std::string());

    bool found = false;

    if (!recursive && wrapscan)
    {
      recursive = true;
      found     = find_next(text, find_flags, forward);
      recursive = false;

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
    if (!recursive)
    {
      log::status(std::string());
    }

    recursive = false;

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
