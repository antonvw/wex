////////////////////////////////////////////////////////////////////////////////
// Name:      vim.cpp
// Purpose:   Implementation of wex::vim
//            https://vim.rtorr.com/
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ctags/ctags.h>
#include <wex/factory/stc-undo.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>

#include <wx/app.h>

#include <algorithm>

#include "vim.h"

namespace wex
{
std::string reverse(const std::string& text)
{
  std::string s(text);
  std::ranges::transform(
    s,
    std::begin(s),
    [](const auto& c)
    {
      return std::islower(c) ? std::toupper(c) : std::tolower(c);
    });
  return s;
}
} // namespace wex

wex::vim::vim(wex::vi* vi, std::string& command, vi::motion_t t)
  : m_vi(vi)
  , m_command(command)
  , m_motion(t)
{
}

bool wex::vim::command_motion(int start_pos)
{
  if (!m_vi->get_stc()->is_visual())
  {
    return false;
  }

  stc_undo undo(
    m_vi->get_stc(),
    stc_undo::undo_t().set(stc_undo::UNDO_ACTION).set(stc_undo::UNDO_POS));

  if (const auto end_pos = m_vi->get_stc()->GetCurrentPos();
      end_pos - start_pos > 0)
  {
    m_vi->get_stc()->SetSelection(start_pos, end_pos);
  }
  else
  {
    m_vi->get_stc()->SetSelection(end_pos, start_pos);
  }

  switch (m_motion)
  {
    case vi::motion_t::G_tilde:
      m_vi->get_stc()->ReplaceSelection(
        reverse(m_vi->get_stc()->get_selected_text()));
      break;

    case vi::motion_t::G_u:
      m_vi->get_stc()->LowerCase();
      break;

    case vi::motion_t::G_U:
      m_vi->get_stc()->UpperCase();
      break;

    default:
      assert(0);
  }

  m_vi->get_stc()->SelectNone();

  return true;
}

bool wex::vim::command_special()
{
  switch (m_motion)
  {
    case vi::motion_t::G_a:
      m_vi->get_stc()->show_ascii_value();
      break;

    case vi::motion_t::G_d:
      ctags::find(
        m_vi->get_stc()->get_word_at_pos(m_vi->get_stc()->GetCurrentPos()),
        m_vi->get_stc());
      break;

    case vi::motion_t::G_f:
      m_vi->get_stc()->link_open();
      break;

    case vi::motion_t::G_star:
    case vi::motion_t::G_hash:
      find_replace_data::get()->set_find_string(
        m_vi->get_stc()->get_word_at_pos(m_vi->get_stc()->GetCurrentPos()));
      m_vi->reset_search_flags();
      m_vi->get_stc()->find(
        find_replace_data::get()->get_find_string(),
        m_vi->search_flags(),
        m_motion == vi::motion_t::G_star);
      break;

    case vi::motion_t::G_t:
    case vi::motion_t::G_T:
      if (auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
          frame != nullptr)
      {
        m_motion == vi::motion_t::G_t ? frame->page_next() : frame->page_prev();
      }
      break;

    default:
      assert(0);
  }

  m_command.erase(0, 2);

  return true;
}

wex::vi::motion_t wex::vim::get_motion(const std::string& command)
{
  if (command.size() == 1)
  {
    return wex::vi::motion_t::G;
  }

  switch (command[1])
  {
    case 'a':
      return wex::vi::motion_t::G_a;

    case 'd':
      return wex::vi::motion_t::G_d;

    case 'f':
      return wex::vi::motion_t::G_f;

    case '*':
      return wex::vi::motion_t::G_star;

    case '#':
      return wex::vi::motion_t::G_hash;

    case 't':
      return wex::vi::motion_t::G_t;

    case 'T':
      return wex::vi::motion_t::G_T;

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

bool wex::vim::is_motion() const
{
  return m_motion > vi::motion_t::G_motion_start &&
         m_motion < vi::motion_t::G_motion_end;
}

bool wex::vim::is_special() const
{
  return m_motion > vi::motion_t::G_special_start &&
         m_motion < vi::motion_t::G_special_end;
}

bool wex::vim::is_vim() const
{
  return m_motion >= vi::motion_t::G;
}

bool wex::vim::motion(int start_pos, size_t& parsed, const vi::function_t& f)
{
  if (!is_motion())
  {
    return false;
  }

  if ((parsed = f(m_command)) == 0)
  {
    return false;
  }

  return command_motion(start_pos);
}

void wex::vim::motion_prep()
{
  if (is_motion() && m_command.size() > 2)
  {
    m_command.erase(0, 2);
  }
}

bool wex::vim::special()
{
  if (is_special() && m_command.size() == 2)
  {
    command_special();
    return true;
  }

  return false;
}
