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
  , m_Type([](const std::string& type) {
      long id = (
        type.empty() || 
       (type.find("popup") == std::string::npos && 
        type.find("main") == std::string::npos)) ? 
        MENU_COMMAND_IS_POPUP | MENU_COMMAND_IS_MAIN: MENU_COMMAND_IS_NONE;

      id |= (type.find("popup") != std::string::npos ? 
        MENU_COMMAND_IS_POPUP: MENU_COMMAND_IS_NONE);
      id |= (type.find("main") != std::string::npos ? 
        MENU_COMMAND_IS_MAIN: MENU_COMMAND_IS_NONE);
      id |= (type.find("separator") != std::string::npos ? 
        MENU_COMMAND_SEPARATOR: MENU_COMMAND_IS_NONE);
      id |= (type.find("ellipses") != std::string::npos ? 
        MENU_COMMAND_ELLIPSES: MENU_COMMAND_IS_NONE);

      return id;} (type))
{
}
  
const std::string wxExMenuCommand::GetCommand(long type) const
{
  auto command = m_Command;

  if (m_SubMenuIsCommand && 
     (type & COMMAND_INCLUDE_SUBCOMMAND))
  {
    command += " " + m_SubMenu;
  }

  if (command.find("&") != std::string::npos && 
    !(type & COMMAND_INCLUDE_ACCELL))
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
