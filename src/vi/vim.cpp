////////////////////////////////////////////////////////////////////////////////
// Name:      vim.cpp
// Purpose:   Implementation of command_g
//            http://www.viemu.com/vi-vim-cheat-sheet.gif
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/stc.h>

#include <algorithm>
#include <string>

#include "motion.h"
#include "vim.h"

namespace wex
{
std::string reverse(const std::string& text)
{
  std::string s(text);
  std::transform(
    std::begin(s),
    std::end(s),
    std::begin(s),
    [](auto& c)
    {
      return std::islower(c) ? std::toupper(c) : std::tolower(c);
    });
  return s;
}
} // namespace wex

bool wex::command_g(vi* vi, vi::motion_t t, int start)
{
  if (!vi->get_stc()->is_visual())
  {
    return false;
  }

  bool ok = true;

  vi->get_stc()->BeginUndoAction();
  vi->get_stc()->position_save();

  if (const auto end = vi->get_stc()->GetCurrentPos(); end - start > 0)
  {
    vi->get_stc()->SetSelection(start, end);
  }
  else
  {
    vi->get_stc()->SetSelection(end, start);
  }

  switch (t)
  {
    case vi::motion_t::G_tilde:
      vi->get_stc()->ReplaceSelection(
        reverse(vi->get_stc()->get_selected_text()));
      break;

    case vi::motion_t::G_u:
      vi->get_stc()->LowerCase();
      break;

    case vi::motion_t::G_U:
      vi->get_stc()->UpperCase();
      break;

    default:
      ok = false;
      break;
  }

  vi->get_stc()->EndUndoAction();
  vi->get_stc()->SelectNone();
  vi->get_stc()->position_restore();

  return ok;
}

wex::vi::motion_t wex::command_g_motion(const std::string& command)
{
  if (command.size() == 1)
  {
    return wex::vi::motion_t::G;
  }
  else
  {
    switch (command[1])
    {
      case 'U':
        return wex::vi::motion_t::G_U;

      case 'u':
        return wex::vi::motion_t::G_u;

      case '~':
        return wex::vi::motion_t::G_tilde;

      default:
        return wex::vi::motion_t::G;
    }
  }
}
