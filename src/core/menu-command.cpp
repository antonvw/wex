////////////////////////////////////////////////////////////////////////////////
// Name:      menu-command.cpp
// Purpose:   Implementation of wex::menu_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/menu-command.h>

#include <algorithm>

wex::menu_command::menu_command(const pugi::xml_node& node)
  : m_command(boost::algorithm::trim_copy(std::string(node.text().get())))
  , m_control(node.attribute("control").value())
  , m_flags(node.attribute("flags").value())
  , m_submenu(
      !node.attribute("submenu").empty() ? node.attribute("submenu").value() :
                                           node.attribute("subcommand").value())
  , m_submenu_is_command(!node.attribute("subcommand").empty())
  , m_text(node.attribute("menu").value())
  , m_type(
      [](const pugi::xml_node& node)
      {
        const std::string text(node.attribute("type").value());
        type_t            id;
        if (text.empty() || (!text.contains("popup") && !text.contains("main")))
          id.set(IS_POPUP).set(IS_MAIN);
        if (text.contains("popup"))
          id.set(IS_POPUP);
        if (text.contains("main"))
          id.set(IS_MAIN);
        if (text.contains("separator"))
          id.set(SEPARATOR);
        if (text.contains("ellipses"))
          id.set(ELLIPSES);
        if (text.contains("is-lines"))
          id.set(IS_LINES);
        if (text.contains("is-selected"))
          id.set(IS_SELECTED);
        if (text.contains("is-visual"))
          id.set(IS_VISUAL);

        return id;
      }(node))
{
}

const std::string wex::menu_command::get_command(include_t type) const
{
  auto command = m_command;

  if (m_submenu_is_command && (type[INCLUDE_SUBCOMMAND]))
  {
    command += " " + m_submenu;
  }

  if (command.contains("&") && !type[INCLUDE_ACCELL])
  {
    command.erase(
      std::remove(command.begin(), command.end(), '&'),
      command.end());
  }

  return command;
}

bool wex::menu_command::use_subcommand() const
{
  return is_help() || (m_submenu_is_command && !m_submenu.empty());
}
