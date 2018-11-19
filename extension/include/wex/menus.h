////////////////////////////////////////////////////////////////////////////////
// Name:      menus.h
// Purpose:   Declaration of wex::menus class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/config.h>
#include <wex/menu.h>
#include <wex/menucommand.h>
#include <wex/util.h>

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
      T& commands) {
      for (const auto& child: node.children())
      {
        if (strcmp(child.name(), "commands") == 0)
        {
          AddCommand(child, commands);
        }
      }
      return commands.size();};

    /// Builds menu from vector of menu commands.
    /// Returns number of items added to menu.
    template <typename T>
    static int build_menu(
      /// the commands
      const T& commands, 
      /// base id for command
      int base_id, 
      /// menu to build
      menu* menu, 
      /// is menu used as popup or main menu
      bool is_popup) {
      wex::menu* submenu = nullptr;
      const std::string unused = "XXXXX";  
      std::string prev_menu = unused;
      int i = 0;

      for (const auto& it : commands)
      {
        bool add = false;
        if (it.type().test(menu_command::IS_POPUP) &&
            it.type().test(menu_command::IS_MAIN))
          add = true;
        else if (it.type().test(menu_command::IS_POPUP))
          add = is_popup;
        else if (it.type().test(menu_command::IS_MAIN))
          add = !is_popup;

        if (add)
        {
          if (!it.get_submenu().empty() && prev_menu != it.get_submenu())
          {
            submenu = new wex::menu();
            prev_menu = it.get_submenu();
            menu->append_separator();
            menu->append_submenu(submenu, it.get_submenu());
          }
          else if (it.get_submenu().empty())
          {
            if (prev_menu != unused)
            {
              prev_menu = unused;
              menu->append_separator();
            }
            submenu = nullptr;
          }

          wex::menu* usemenu = (submenu == nullptr ? menu: submenu);
          usemenu->append(
            base_id + i, 
            ellipsed(
              it.get_command(menu_command::INCLUDE_ACCELL),
              std::string(),
              (it.type().test(menu_command::ELLIPSES)) > 0));

          if ((it.type().test(menu_command::SEPARATOR)) > 0)
          {
            usemenu->append_separator();
          }
        }
        i++;
      }

      return menu->GetMenuItemCount();};
    
    /// Returns the xml filename.
    static const path get_filename() {
      return path(config().dir(), "menus.xml");};
    
    /// Loads entries from xml document.
    /// Returns false if document could not be loaded, or
    /// no entries were added.
    template <typename T>
    static bool load(const std::string& name, T& entries) {
      pugi::xml_document doc;
      if (!get_filename().file_exists() ||
          !doc.load_file(
             get_filename().data().string().c_str(),
             pugi::parse_default | pugi::parse_trim_pcdata))
      {
        return false;
      }
      entries.clear();
      for (const auto& child: doc.document_element().children())
      {
        if (strcmp(child.name(), name.c_str()) == 0)
        {
          entries.push_back({child});
        }
      }
      return !entries.empty();};
  private:
    template <typename T>
    static void AddCommand(const pugi::xml_node& node, T& commands)
    {
      for (const auto& child: node.children())
      {
        if (strcmp(child.name(), "command") == 0)
        {
          commands.push_back(
            {child.text().get(),
             child.attribute("type").value(), 
             child.attribute("submenu").value(), 
             child.attribute("subcommand").value(),
             child.attribute("flags").value()});
        }
      }};
  };
};
