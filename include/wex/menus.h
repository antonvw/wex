////////////////////////////////////////////////////////////////////////////////
// Name:      menus.h
// Purpose:   Declaration of wex::menus class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/menu-command.h>
#include <wex/menu.h>

namespace wex
{
/// This class offers methods to handle menu commands.
class menus
{
public:
  /// Adds commands from xml to vector of menu commands.
  /// Returns number of commands added.
  template <typename T>
  static auto add_commands(
    /// node with data
    const pugi::xml_node& node,
    /// the commands to be filled
    T& commands)
  {
    for (const auto& child : node.children())
    {
      if (strcmp(child.name(), "commands") == 0)
      {
        add_command(child, commands);
      }
    }
    return commands.size();
  };

  /// Builds menu from vector of menu commands.
  /// Returns number of items added to menu.
  template <typename T>
  static size_t build_menu(
    /// the commands
    const T& commands,
    /// base id for command
    int base_id,
    /// menu to build
    menu* menu)
  {
    wex::menu*        submenu   = nullptr;
    const std::string unused    = "XXXXX";
    std::string       prev_menu = unused;
    int               i         = 0;

    for (const auto& it : commands)
    {
      bool add = false;
      if (
        it.type().test(menu_command::IS_POPUP) &&
        it.type().test(menu_command::IS_MAIN))
        add = true;
      else if (it.type().test(menu_command::IS_POPUP))
        add = menu->style().test(menu::IS_POPUP);
      else if (it.type().test(menu_command::IS_MAIN))
        add = !menu->style().test(menu::IS_POPUP);

      if (
        (menu->style().test(menu::IS_SELECTED) &&
         !it.type().test(menu_command::IS_SELECTED)) ||
        (!menu->style().test(menu::IS_SELECTED) &&
         it.type().test(menu_command::IS_SELECTED)))
      {
        add = false;
      }

      if (
        !menu->style().test(menu::IS_VISUAL) &&
        it.type().test(menu_command::IS_VISUAL))
      {
        add = false;
      }

      if (add)
      {
        if (!it.submenu().empty() && prev_menu != it.submenu())
        {
          submenu   = new wex::menu();
          prev_menu = it.submenu();
          menu->append({{}, {submenu, it.submenu()}});
        }
        else if (it.submenu().empty())
        {
          if (prev_menu != unused)
          {
            prev_menu = unused;
            menu->append({{}});
          }
          submenu = nullptr;
        }

        wex::menu* usemenu = (submenu == nullptr ? menu : submenu);
        usemenu->append(
          {{base_id + i,
            ellipsed(
              it.text().empty() ? it.get_command(menu_command::INCLUDE_ACCELL) :
                                  it.text(),
              it.control(),
              it.type().test(menu_command::ELLIPSES))}});

        if (it.type().test(menu_command::SEPARATOR))
        {
          usemenu->append({{}});
        }
      }
      i++;
    }

    return menu->GetMenuItemCount();
  };

  /// Returns the xml filename.
  static const path get_filename()
  {
    return path(config::dir(), "wex-menus.xml");
  };

  /// Loads entries from xml document.
  /// Returns false if document could not be loaded, or
  /// no entries were added.
  template <typename T> static bool load(const std::string& name, T& entries)
  {
    pugi::xml_document doc;
    if (
      !get_filename().file_exists() ||
      !doc.load_file(
        get_filename().string().c_str(),
        pugi::parse_default | pugi::parse_trim_pcdata))
    {
      return false;
    }
    entries.clear();
    for (const auto& child : doc.document_element().children())
    {
      if (strcmp(child.name(), name.c_str()) == 0)
      {
        entries.push_back({child});
      }
    }
    return !entries.empty();
  };

private:
  template <typename T>
  static void add_command(const pugi::xml_node& node, T& commands)
  {
    for (const auto& child : node.children())
    {
      if (strcmp(child.name(), "command") == 0)
      {
        commands.push_back({child});
      }
    }
  };
};
}; // namespace wex
