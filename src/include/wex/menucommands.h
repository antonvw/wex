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
      : m_Name(name)
      , m_Commands(commands) {;};
    
    /// Constructor using xml node.
    menu_commands(const pugi::xml_node& node) 
      : m_Name(node.attribute("name").value()) {
      if (m_Name.empty())
      {
        log("no name") << node;
      }
      else
      {
        if (menus::add_commands(node, m_Commands) == 0)
        {
          log("no commands found for") << m_Name;
        }  
      }};
    
    /// Returns the menu command equal to name specified.  
    /// or empty command if name could not be found.
    const T find_command(const std::string& name) const {
      for (const auto& i : m_Commands)
      {
        if (i.get_command() == name)
        {
          return i;
        }
      }
      return T();};
    
    /// Returns the flags key.
    const auto & flags_key() const {return m_FlagsKey;};

    /// Returns the current command.  
    const auto get_command() const {
      return m_Commands.empty() ? T(): m_Commands.at(m_CommandIndex);};

    /// Returns all the commands.
    const auto & get_commands() const {return m_Commands;};

    /// Returns the name for this group of commands.
    const auto & name() const {return m_Name;};

    /// Sets the current command.
    /// Returns true if command was set.
    bool set_command(
      /// a command no from commands
      int command_no) {
      if (command_no < 0 || command_no >= (int)m_Commands.size())
      {
        return false;
      }
      m_CommandIndex = command_no;
      m_FlagsKey = "menuflags/" + m_Name + std::to_string(m_CommandIndex);
      return true;};
  private:
    int m_CommandIndex = 0;

    std::string m_FlagsKey;
    std::string m_Name;
    std::vector < T > m_Commands;
  };
};
