////////////////////////////////////////////////////////////////////////////////
// Name:      command-parse.cpp
// Purpose:   Implementation of class wex::vi::parse_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/stc.h>
#include <wex/vi/macros.h>
#include <wex/vi/vi.h>

#include "motion.h"

bool wex::vi::parse_command(std::string& command)
{
  const auto org(command);

  if (const auto& it = get_macros().get_keys_map().find(command.front());
      it != get_macros().get_keys_map().end())
  {
    command = it->second;
  }

  m_count = 1;

  if (command.front() == '"')
  {
    if (command.size() < 2)
      return false;
    set_register(command[1]);
    get_macros().record(command);
    command.erase(0, 2);
  }
  else if (command.front() == ':')
  {
    return ex::command(command);
  }
  else
  {
    filter_count(command);
  }

  if (command.empty())
  {
    return false;
  }

  const motion_t motion = get_motion(command);

  bool check_other = true;

  switch (command.size())
  {
    case 1:
      if (
        m_mode.is_visual() && !get_stc()->GetSelectedText().empty() &&
        (motion == motion_t::CHANGE || motion == motion_t::DEL ||
         motion == motion_t::YANK))
      {
        if (motion == motion_t::CHANGE)
        {
          m_mode.escape();
          std::string s("i");
          m_mode.transition(s);
          command.erase(0, 1);
        }
        else
        {
          command.erase(0, 1);
          m_mode.escape();
        }
      }
      else if (m_mode.transition(command))
      {
        check_other = false;
      }
      else
      {
        m_insert_command.clear();
      }
      break;

    default:
      if (other_command(command))
      {
        return true;
      }

      parse_command_motion(motion, command, check_other);
  }

  if (
    check_other && !motion_command(motion, command) && !other_command(command))
  {
    return false;
  }
  else
  {
    set_register(0);
  }

  if (!command.empty())
  {
    if (m_mode.is_insert())
    {
      return insert_mode(command);
    }
    else if (command != org)
    {
      return parse_command(command);
    }
    else
    {
      return false;
    }
  }

  return true;
}

void wex::vi::parse_command_motion(
  motion_t     motion,
  std::string& command,
  bool&        check_other)
{
  if (motion > motion_t::G_aa && motion < motion_t::G_ZZ)
  {
    if (command.size() > 2)
    {
      command.erase(0, 2);
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
}
