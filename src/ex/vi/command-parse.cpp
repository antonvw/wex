////////////////////////////////////////////////////////////////////////////////
// Name:      command-parse.cpp
// Purpose:   Implementation of class wex::vi::parse_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/ex/macros.h>
#include <wex/syntax/stc.h>
#include <wex/vi/vi.h>

#include "vim.h"

bool wex::vi::parse_command(std::string& command)
{
  if (const auto& it = get_macros().get_keys_map().find(command.front());
      it != get_macros().get_keys_map().end())
  {
    command = it->second;
  }

  if (
    m_command.type() != ex_command::type_t::FIND &&
    m_command.type() != ex_command::type_t::FIND_MARGIN)
  {
    m_count = 1;
  }

  // if this is a register, wait for next char for register name
  if (command.front() == '"')
  {
    if (command.size() < 2)
    {
      return false;
    }
    set_register(command[1]);
    command.erase(0, 2);
    return true;
  }

  if (command.front() == ':')
  {
    return ex::command(command);
  }

  filter_count(command);

  if (command.empty())
  {
    return false;
  }

  return parse_command_handle(command);
}

bool wex::vi::parse_command_handle(std::string& command)
{
  const motion_t motion      = get_motion(command);
  bool           check_other = true;

  if (command.size() == 1)
  {
    if (parse_command_handle_single(motion, command, check_other))
    {
      return true;
    }
  }
  else if (
    other_command(command) ||
    parse_command_motion(motion, command, check_other))
  {
    return true;
  }

  if (
    check_other && !motion_command(motion, command) && !other_command(command))
  {
    return false;
  }

  set_register(0);

  if (!command.empty())
  {
    if (m_mode.is_insert())
    {
      return insert_mode(command);
    }
    if (command != m_command_string)
    {
      return parse_command(command);
    }

    return false;
  }

  return true;
}

bool wex::vi::parse_command_handle_single(
  motion_t     motion,
  std::string& command,
  bool&        check_other)
{
  if (
    (m_mode.is_visual() || command == "d") &&
    !get_stc()->get_selected_text().empty() &&
    (motion == motion_t::CHANGE || motion == motion_t::DEL ||
     motion == motion_t::YANK))
  {
    if (motion == motion_t::CHANGE)
    {
      std::string s("i");
      m_mode.transition(s);
      command.erase(0, 1);
      return true;
    }

    command.erase(0, 1);
    m_mode_yank = m_mode.get();
    m_mode.escape();
  }
  else if (m_mode.transition(command))
  {
    check_other = false;
  }
  else
  {
    m_insert_command.clear();
  }

  return false;
}

bool wex::vi::parse_command_motion(
  motion_t     motion,
  std::string& command,
  bool&        check_other)
{
  if (wex::vim vim(this, command, motion); vim.is_vim())
  {
    if (vim.special())
    {
      return true;
    }
    else if (!vim.is_motion() && command.size() > 1)
    {
      bell();
      command.clear();
      return true;
    }
  }
  else
  {
    switch (motion)
    {
      case motion_t::CHANGE:
        m_mode.transition(command);
        break;

      case motion_t::DEL:
      case motion_t::YANK:
        command.erase(0, 1);
        break;

      case motion_t::NAVIGATE:
        if (m_mode.transition(command))
        {
          check_other = false;
        }
        else
        {
          m_insert_command.clear();
        }
        break;

      default:
        // do nothing
        break;
    }
  }

  return false;
}
