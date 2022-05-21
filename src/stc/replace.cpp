////////////////////////////////////////////////////////////////////////////////
// Name:      replace.cpp
// Purpose:   Implementation of class wex::stc replace methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/factory/stc-undo.h>
#include <wex/stc/stc.h>
#include <wex/ui/frd.h>

int wex::stc::replace_all(
  const std::string& find_text,
  const std::string& replace_text)
{
  int selection_from_end = 0;

  if (
    SelectionIsRectangle() ||
    get_number_of_lines(GetSelectedText().ToStdString()) > 1)
  {
    TargetFromSelection();
    selection_from_end = GetLength() - GetTargetEnd();
  }
  else
  {
    TargetWholeDocument();
  }

  int nr_replacements = 0;
  set_search_flags(-1);
  stc_undo undo(this);

  while (SearchInTarget(find_text) != -1)
  {
    bool skip_replace = false;

    // Check that the target is within the rectangular selection.
    // If not just continue without replacing.
    if (SelectionIsRectangle())
    {
      const auto line      = LineFromPosition(GetTargetStart());
      const auto start_pos = GetLineSelStartPosition(line);
      const auto end_pos   = GetLineSelEndPosition(line);
      const auto length    = GetTargetEnd() - GetTargetStart();

      if (
        start_pos == wxSTC_INVALID_POSITION ||
        end_pos == wxSTC_INVALID_POSITION || GetTargetStart() < start_pos ||
        GetTargetStart() + length > end_pos)
      {
        skip_replace = true;
      }
    }

    if (!skip_replace)
    {
      if (is_hexmode())
      {
        m_hexmode.replace_target(replace_text);
      }
      else
      {
        find_replace_data::get()->is_regex() ? ReplaceTargetRE(replace_text) :
                                               ReplaceTarget(replace_text);
      }

      nr_replacements++;
    }

    SetTargetRange(GetTargetEnd(), GetLength() - selection_from_end);

    if (GetTargetStart() >= GetTargetEnd())
    {
      break;
    }
  }

  log::status(_("Replaced"))
    << nr_replacements << "occurrences of" << find_text;

  return nr_replacements;
}

bool wex::stc::replace_next(bool stc_find_string)
{
  return replace_next(
    find_replace_data::get()->get_find_string(),
    find_replace_data::get()->get_replace_string(),
    -1,
    stc_find_string);
}

bool wex::stc::replace_next(
  const std::string& find_text,
  const std::string& replace_text,
  int                find_flags,
  bool               stc_find_string)
{
  if (stc_find_string && !GetSelectedText().empty())
  {
    TargetFromSelection();
  }
  else
  {
    SetTargetRange(GetCurrentPos(), GetLength());
    set_search_flags(find_flags);
    if (SearchInTarget(find_text) == -1)
      return false;
  }

  if (is_hexmode())
  {
    m_hexmode.replace_target(replace_text);
  }
  else
  {
    find_replace_data::get()->is_regex() ? ReplaceTargetRE(replace_text) :
                                           ReplaceTarget(replace_text);
  }

  find(find_text, find_flags);

  return true;
}
