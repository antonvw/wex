////////////////////////////////////////////////////////////////////////////////
// Name:      menus.h
// Purpose:   Declaration of wxExMenus class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wx/extension/menu.h>
#include <wx/extension/menucommand.h>
#include <wx/extension/util.h>

/// This class offers methods to handle menu commands.
class WXDLLIMPEXP_BASE wxExMenus
{
public:
  /// Adds commands from xml to vector of menu commands.
  /// Returns number of commands added.
  template <typename T>
  static auto AddCommands(
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
  static int BuildMenu(
    /// the commands
    const T& commands, 
    /// base id for command
    int base_id, 
    /// menu to build
    wxExMenu* menu, 
    /// is menu used as popup or main menu
    bool is_popup) {
    wxExMenu* submenu = nullptr;
    const std::string unused = "XXXXX";  
    std::string prev_menu = unused;
    int i = 0;

    for (const auto& it : commands)
    {
      bool add = false;
      if (it.GetType() & wxExMenuCommand::MENU_COMMAND_IS_POPUP &&
          it.GetType() & wxExMenuCommand::MENU_COMMAND_IS_MAIN)
        add = true;
      else if (it.GetType() & wxExMenuCommand::MENU_COMMAND_IS_POPUP)
        add = is_popup;
      else if (it.GetType() & wxExMenuCommand::MENU_COMMAND_IS_MAIN)
        add = !is_popup;

      if (add)
      {
        if (!it.GetSubMenu().empty() && prev_menu != it.GetSubMenu())
        {
          submenu = new wxExMenu();
          prev_menu = it.GetSubMenu();
          menu->AppendSeparator();
          menu->AppendSubMenu(submenu, it.GetSubMenu());
        }
        else if (it.GetSubMenu().empty())
        {
          if (prev_menu != unused)
          {
            prev_menu = unused;
            menu->AppendSeparator();
          }
          submenu = nullptr;
        }

        wxExMenu* usemenu = (submenu == nullptr ? menu: submenu);
        usemenu->Append(
          base_id + i, 
          wxExEllipsed(
            it.GetCommand(false, true), // use no sub and do accel
            std::string(),
            (it.GetType() & wxExMenuCommand::MENU_COMMAND_ELLIPSES) > 0));

        if ((it.GetType() & wxExMenuCommand::MENU_COMMAND_SEPARATOR) > 0)
        {
          usemenu->AppendSeparator();
        }
      }
      i++;
    }

    return menu->GetMenuItemCount();};
  
  /// Returns the xml filename.
  static const wxExPath GetFileName() {
    return wxExPath(wxExConfigDir(), "menus.xml");};
  
  /// Loads entries from xml document.
  /// Returns false if document could not be loaded, or
  /// no entries were added.
  template <typename T>
  static bool Load(const std::string& name, T& entries) {
    pugi::xml_document doc;
    if (!GetFileName().FileExists() ||
        !doc.load_file(
           GetFileName().Path().string().c_str(),
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
