////////////////////////////////////////////////////////////////////////////////
// Name:      menu-commands.h
// Purpose:   Declaration of class wex::menu_commands
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

#include <pugixml.hpp>

#include <wex/ui/menus.h>

namespace wex
{
/// This class offers a collection (vector) of menu commands,
/// where the exact type of each command is templatized.
template <class T> class menu_commands
{
public:
  /// Default constructor.
  menu_commands(
    const std::string&    name     = std::string(),
    const std::vector<T>& commands = {})
    : m_name(name)
    , m_commands(commands)
  {
    ;
  };

  /// Constructor using xml node.
  menu_commands(const pugi::xml_node& node)
    : m_name(node.attribute("name").value())
  {
    if (!m_name.empty())
    {
      menus::add_commands(node, m_commands);
    }
  };

  /// Returns the menu command equal to name specified.
  /// or empty command if name could not be found.
  const T find(const std::string& name) const
  {
    if (const auto& it = std::find_if(
          m_commands.begin(),
          m_commands.end(),
          [name](const auto& p)
          {
            return name == p.get_command();
          });
        it != m_commands.end())
    {
      return *it;
    }
    return T();
  };

  /// Returns the flags key.
  const auto& flags_key() const { return m_flags_key; }

  /// Returns the current command.
  const auto get_command() const
  {
    return m_commands.empty() ? T() : m_commands.at(m_command_index);
  };

  /// Returns all the commands.
  const auto& get_commands() const { return m_commands; }

  /// Returns the name for this group of commands.
  const auto& name() const { return m_name; }

  /// Sets the current command.
  /// Returns true if command was set.
  bool set_command(
    /// a command no from commands
    int command_no)
  {
    if (command_no < 0 || command_no >= static_cast<int>(m_commands.size()))
    {
      return false;
    }
    m_command_index = command_no;
    m_flags_key     = "menu.flags/" + m_name + std::to_string(m_command_index);
    return true;
  };

private:
  int m_command_index{0};

  std::string m_flags_key, m_name;

  std::vector<T> m_commands;
};
}; // namespace wex
