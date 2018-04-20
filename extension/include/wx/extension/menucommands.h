////////////////////////////////////////////////////////////////////////////////
// Name:      menucommands.h
// Purpose:   Declaration of class wxExMenuCommands
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <pugixml.hpp>
#include <wx/extension/menus.h>
#include <wx/extension/log.h>

/// This class offers a collection (vector) of menu commands,
/// where the exact type of each command is templatized.
template <class T> 
class WXDLLIMPEXP_BASE wxExMenuCommands
{
public:
  /// Default constructor.
  wxExMenuCommands(
    const std::string& name = std::string(),
    const std::vector < T > & commands = {}) 
    : m_Name(name)
    , m_Commands(commands) {;};
  
  /// Constructor using xml node.
  wxExMenuCommands(const pugi::xml_node& node) 
    : m_Name(node.attribute("name").value()) {
    if (m_Name.empty())
    {
      wxExLog("no name") << node;
    }
    else
    {
      if (wxExMenus::AddCommands(node, m_Commands) == 0)
      {
        wxExLog("no commands found for") << m_Name;
        m_Commands.push_back({});
      }  
    }};
  
  /// Returns the current command.  
  const auto & GetCommand() const {
    return m_Commands.at(m_CommandIndex);};

  /// Returns the commands.
  const auto & GetCommands() const {return m_Commands;};

  /// Returns the flags key.
  const auto & GetFlagsKey() const {return m_FlagsKey;};

  /// Returns the name for this group of commands.
  const auto & GetName() const {return m_Name;};

  /// Sets the current command.
  /// Returns true if command was set.
  bool SetCommand(
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
