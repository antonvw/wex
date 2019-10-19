////////////////////////////////////////////////////////////////////////////////
// Name:      menu_commands.h
// Purpose:   Declaration of class wex::menu_commands
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <pugixml.hpp>
#include <wex/menus.h>
#include <wex/log.h>

namespace wex
{
  /// This class offers a collection (vector) of menu commands,
  /// where the exact type of each command is templatized.
  template <class T> 
  class menu_commands
  {
  public:
    /// Default constructor.
    menu_commands(
      const std::string& name = std::string(),
      const std::vector < T > & commands = {}) 
      : m_name(name)
      , m_commands(commands) {;};
    
    /// Constructor using xml node.
    menu_commands(const pugi::xml_node& node) 
      : m_name(node.attribute("name").value()) {
      if (m_name.empty())
      {
        log("no name") << node;
      }
      else
      {
        if (menus::add_commands(node, m_commands) == 0)
        {
          log("no commands found for") << m_name;
        }  
      }};
    
    /// Returns the menu command equal to name specified.  
    /// or empty command if name could not be found.
    const T find_command(const std::string& name) const {
      for (const auto& i : m_commands)
      {
        if (i.get_command() == name)
        {
          return i;
        }
      }
      return T();};
    
    /// Returns the flags key.
    const auto & flags_key() const {return m_flags_key;};

    /// Returns the current command.  
    const auto get_command() const {
      return m_commands.empty() ? T(): m_commands.at(m_command_index);};

    /// Returns all the commands.
    const auto & get_commands() const {return m_commands;};

    /// Returns the name for this group of commands.
    const auto & name() const {return m_name;};

    /// Sets the current command.
    /// Returns true if command was set.
    bool set_command(
      /// a command no from commands
      int command_no) {
      if (command_no < 0 || command_no >= (int)m_commands.size())
      {
        return false;
      }
      m_command_index = command_no;
      m_flags_key = "menuflags/" + m_name + std::to_string(m_command_index);
      return true;};
  private:
    int m_command_index {0};

    std::string 
      m_flags_key,
      m_name;

    std::vector < T > m_commands;
  };
};
