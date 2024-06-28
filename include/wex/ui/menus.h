////////////////////////////////////////////////////////////////////////////////
// Name:      menus.h
// Purpose:   Declaration of wex::menus class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/core/menu-command.h>
#include <wex/ui/menu.h>

namespace wex
{
/// This class offers methods to handle menu commands.
class menus
{
public:
  /// Adds commands from xml to vector of menu commands.
  /// Returns number of commands added.
  template <typename T>
  static size_t add_commands(
    /// node with data
    const pugi::xml_node& node,
    /// the commands to be filled
    T& commands);

  /// Builds menu from vector of menu commands.
  /// Returns number of items added to menu.
  template <typename T>
  static size_t build_menu(
    /// the commands
    const T& commands,
    /// base id for command
    int base_id,
    /// menu to build
    menu* menu);

  /// Loads entries from xml document.
  /// Returns false if document could not be loaded, or
  /// no entries were added.
  template <typename T> static bool load(const std::string& name, T& entries);

  /// Returns the xml filename.
  static const wex::path path();

private:
  template <typename T>
  static void add_command(const pugi::xml_node& node, T& commands);

  static void add_menu(const menu_command& mc, menu* menu);
  static bool allow_add_menu(const menu_command& mc, const menu* menu);
  static bool load_doc(pugi::xml_document& doc);
  static void no_commands_added(const pugi::xml_node& node);

  static inline int m_id{0};
};

// implementation

template <typename T>
size_t wex::menus::add_commands(const pugi::xml_node& node, T& commands)
{
  size_t added = 0;

  for (const auto& child : node.children())
  {
    if (strcmp(child.name(), "commands") == 0)
    {
      add_command(child, commands);
      added++;
    }
  }

  if (added == 0)
  {
    no_commands_added(node);
  }

  return added;
}

template <typename T>
size_t wex::menus::build_menu(const T& commands, int base_id, menu* menu)
{
  m_id = base_id;

  for (const auto& it : commands)
  {
    if (allow_add_menu(it, menu))
    {
      add_menu(it, menu);
    }

    m_id++;
  }

  return menu->GetMenuItemCount();
}

template <typename T>
void wex::menus::add_command(const pugi::xml_node& node, T& commands)
{
  for (const auto& child : node.children())
  {
    if (strcmp(child.name(), "command") == 0)
    {
      commands.push_back({child});
    }
  }
};

template <typename T> bool wex::menus::load(const std::string& name, T& entries)
{
  pugi::xml_document doc;
  if (!load_doc(doc))
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
}
}; // namespace wex
