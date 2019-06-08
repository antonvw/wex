////////////////////////////////////////////////////////////////////////////////
// Name:      menu_command.cpp
// Purpose:   Implementation of wex::menu_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wex/menucommand.h>
#include <wex/util.h>
            
wex::menu_command::menu_command(const pugi::xml_node& node)
  : m_command(skip_white_space(node.text().get()))
  , m_control(node.attribute("control").value())
  , m_flags(node.attribute("flags").value())
  , m_submenu(!node.attribute("submenu").empty() ? 
      node.attribute("submenu").value(): node.attribute("subcommand").value())
  , m_submenu_is_command(!node.attribute("subcommand").empty())
  , m_type([](const pugi::xml_node& node) {
      const std::string text(node.attribute("type").value());
      type_t id;
      if (text.empty() || 
         (text.find("popup") == std::string::npos && 
          text.find("main") == std::string::npos))
        id.set(IS_POPUP).set(IS_MAIN);
      if (text.find("popup") != std::string::npos)
        id.set(IS_POPUP);
      if (text.find("main") != std::string::npos)
        id.set(IS_MAIN);
      if (text.find("separator") != std::string::npos)
        id.set(SEPARATOR);
      if (text.find("ellipses") != std::string::npos)
        id.set(ELLIPSES);

      return id;} (node))
{
}
  
const std::string wex::menu_command::get_command(include_t type) const
{
  auto command = m_command;

  if (m_submenu_is_command && (type[INCLUDE_SUBCOMMAND]))
  {
    command += " " + m_submenu;
  }

  if (command.find("&") != std::string::npos && !type[INCLUDE_ACCELL])
  {
    command.erase(
      std::remove(command.begin(), command.end(), '&'), command.end());
  }

  return command;
}

bool wex::menu_command::use_subcommand() const
{
  return is_help() || (m_submenu_is_command && !m_submenu.empty());
}
