////////////////////////////////////////////////////////////////////////////////
// Name:      menu_command.cpp
// Purpose:   Implementation of wex::menu_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wex/menu.h>
#include <wex/menucommand.h>
#include <wex/util.h>

wex::menu_command::menu_command(
  const std::string& command,
  const std::string& type_text,
  const std::string& submenu,
  const std::string& subcommand,
  const std::string& flags)
  : m_Command(command)
  , m_Flags(flags)
  , m_SubMenu(!submenu.empty() ? submenu: subcommand)
  , m_SubMenuIsCommand(!subcommand.empty())
  , m_Type([](const std::string& text) {
      long id = (
        text.empty() || 
       (text.find("popup") == std::string::npos && 
        text.find("main") == std::string::npos)) ? 
        IS_POPUP | IS_MAIN: IS_NONE;

      id |= (text.find("popup") != std::string::npos ? 
        IS_POPUP: IS_NONE);
      id |= (text.find("main") != std::string::npos ? 
        IS_MAIN: IS_NONE);
      id |= (text.find("separator") != std::string::npos ? 
        SEPARATOR: IS_NONE);
      id |= (text.find("ellipses") != std::string::npos ? 
        ELLIPSES: IS_NONE);

      return id;} (type_text))
{
}
  
const std::string wex::menu_command::GetCommand(long type) const
{
  auto command = m_Command;

  if (m_SubMenuIsCommand && 
     (type & INCLUDE_SUBCOMMAND))
  {
    command += " " + m_SubMenu;
  }

  if (command.find("&") != std::string::npos && 
    !(type & INCLUDE_ACCELL))
  {
    command.erase(
      std::remove(command.begin(), command.end(), '&'), command.end());
  }

  return command;
}

bool wex::menu_command::UseSubcommand() const
{
  return IsHelp() || (m_SubMenuIsCommand && !m_SubMenu.empty());
}
