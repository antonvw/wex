////////////////////////////////////////////////////////////////////////////////
// Name:      menucommand.cpp
// Purpose:   Implementation of wxExMenuCommand class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wx/extension/menucommand.h>
#include <wx/extension/menu.h>
#include <wx/extension/util.h>

wxExMenuCommand::wxExMenuCommand(
  const std::string& command,
  const std::string& type,
  const std::string& submenu,
  const std::string& subcommand,
  const std::string& flags)
  : m_Command(command)
  , m_Flags(flags)
  , m_SubMenu(!submenu.empty() ? submenu: subcommand)
  , m_SubMenuIsCommand(!subcommand.empty())
  , m_Type(From(type))
{
}
  
long wxExMenuCommand::From(const std::string& type) const
{
  long command = (
    type.empty() || 
   (type.find("popup") == std::string::npos && 
    type.find("main") == std::string::npos)) ? 
    MENU_COMMAND_IS_POPUP | MENU_COMMAND_IS_MAIN: MENU_COMMAND_IS_NONE;

  command |= (type.find("popup") != std::string::npos ? MENU_COMMAND_IS_POPUP: 0);
  command |= (type.find("main") != std::string::npos ? MENU_COMMAND_IS_MAIN: 0);
  command |= (type.find("separator") != std::string::npos ? MENU_COMMAND_SEPARATOR: 0);
  command |= (type.find("ellipses") != std::string::npos ? MENU_COMMAND_ELLIPSES: 0);

  return command;
}

const std::string wxExMenuCommand::GetCommand(
  bool include_subcommand,
  bool include_accelerators) const
{
  std::string command = m_Command;

  if (m_SubMenuIsCommand && include_subcommand)
  {
    command += " " + m_SubMenu;
  }

  if (command.find("&") != std::string::npos && !include_accelerators)
  {
    command.erase(
      std::remove(command.begin(), command.end(), '&'), command.end());
  }

  return command;
}

bool wxExMenuCommand::UseSubcommand() const
{
  return IsHelp() || (m_SubMenuIsCommand && !m_SubMenu.empty());
}
