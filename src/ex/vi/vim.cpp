////////////////////////////////////////////////////////////////////////////////
// Name:      vim.cpp
// Purpose:   Implementation of wex::vim
//            https://vim.rtorr.com/
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/ctags/ctags.h>
#include <wex/factory/stc-undo.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>

#include <wx/app.h>

#include <algorithm>
#include <string>

#include "vim.h"

namespace wex
{
int line_to_fold(syntax::stc* stc)
{
  const auto level = stc->GetFoldLevel(stc->get_current_line());

  return 
     (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      stc->get_current_line() :
      stc->GetFoldParent(stc->get_current_line());
}

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

wex::vim::vim(wex::vi* vi, std::string& command)
  : m_vi(vi)
  , m_stc(vi->get_stc())
  , m_command(command)
  , m_command_org(command)
  , m_motion_commands({
    {"g~", [&](const std::string& command) {      
       m_stc->ReplaceSelection(reverse(m_stc->get_selected_text());}},
    {"gu", [&](const std::string& command) {
       m_stc->LowerCase();}},
    {"gU", [&](const std::string& command) {
       m_stc->UpperCase();}}})
  , m_other_commands({
    {"g8", [&](const std::string& command) {      
       m_stc->show_ascii_value(true);}},
    {"g*", [&](const std::string& command) {command_find(command);}}, 
    {"g#", [&](const std::string& command) {command_find(command);}}, 
    {"ga", [&](const std::string& command) {m_stc->show_ascii_value();}}, 
    {"gd", [&](const std::string& command) {ctags::find(m_stc->get_word_at_pos(pos), m_stc);;}}, 
    {"gf", [&](const std::string& command) {m_stc->link_open();;}}, 
    {"gm", [&](const std::string& command) {
      const auto ll(m_stc->LineLength(m_stc->get_current_line()));
      m_vi->command(std::to_string(int(ll / 2)) + "|");}}, 
    {"gt", [&](const std::string& command) {m_vi->frame()->page_next();}}, 
    {"gT", [&](const std::string& command) {m_vi->frame()->page_prev();}},
    {"za", [&](const std::string& command) {      
      m_stc->ToggleFold(line_to_fold(m_stc));}},
    {"zc", [&](const std::string& command) {      
      command_z_fold(command);}},
    {"zo", [&](const std::string& command) {      
      command_z_fold(command);}},
    {"zf", [&](const std::string& command) { 
      m_stc->get_lexer().set_property("fold", "1");
      m_stc->get_lexer().apply();
      m_stc->fold(true);}},
    {"zC",  [&](const std::string& command) { 
      m_stc->fold(true);}},
    {"zE",  [&](const std::string& command) { 
      m_stc->get_lexer().set_property("fold", "0");
      m_stc->get_lexer().apply();
      m_stc->fold(false);}},
    {"zO",  [&](const std::string& command) { 
      for (int i = 0; i < m_stc->get_line_count(); i++)
      {
        m_stc->EnsureVisible(i);
      }}},
    {"zz", [&](const std::string& command) { 
      m_stc->VerticalCentreCaret();}}})
{
}

void wex::vim::command_find(const std::string& command)
{
  const auto pos = m_stc->GetCurrentPos();

  find_replace_data::get()->set_find_string(m_stc->get_word_at_pos(pos));
  m_vi->reset_search_flags();
  m_stc->find(
    find_replace_data::get()->get_find_string(),
    m_vi->search_flags(),
    command[1] == "*");
}

bool wex::vim::command_motion(int start_pos)
{
  if (!m_stc->is_visual())
  {
    return false;
  }

  stc_undo undo(
    m_stc,
    stc_undo::undo_t().set(stc_undo::UNDO_ACTION).set(stc_undo::UNDO_POS));

  if (const auto end_pos = m_stc->GetCurrentPos(); end_pos - start_pos > 0)
  {
    m_stc->SetSelection(start_pos, end_pos);
  }
  else
  {
    m_stc->SetSelection(end_pos, start_pos);
  }

  m_stc->SelectNone();

  if (const auto& a = std::ranges::find_if(
    m_motion_commands,
    [&](auto const& i)
    {
      return i == m_command.substr(0, 2);
    }) != m_motion_commands.end())
  {
    it->second();
  }

  return true;
}

bool wex::vim::command_other()
{
  if (const auto& it = find_from<vi::commands_t>(m_other_commands, m_command);
    it != m_other_commands.end())
  {
    it->second();
  }

  m_command.erase(0, 2);

  return true;
}

void wex::vim::command_z_fold(const std::string& command)
{
  if (
    (m_stc->GetFoldExpanded(line_to_fold) && 
       boost::algorithm::trim_copy(m_command) == "zc") ||
    (!m_stc->GetFoldExpanded(line_to_fold) &&
       boost::algorithm::trim_copy(m_command) == "zo"))
  {
    m_stc->ToggleFold(line_to_fold);
  }
}

bool wex::vim::is_motion() const
{
  const auto& it = find_from<vi::commands_t>(m_motion_commands, m_command);
  return it != m_motion_commands.end();
}

bool wex::vim::is_other() const
{
  return !is_motion();
}

bool wex::vim::is_vim() const
{
  return !m_command_org.empty() && (m_command_org[0] == 'g' || m_command_org[0] == 'z');
}

bool wex::vim::motion(int start_pos, size_t& parsed, const vi::function_t& f)
{
  if (!is_motion())
  {
    return false;
  }

  if ((parsed = f(m_command.substr(2))) == 0)
  {
    return false;
  }

  return command_motion(start_pos);
}

bool wex::vim::other()
{
  if (is_other() && m_command.size() == 2)
  {
    command_other();
    return true;
  }

  return false;
}
