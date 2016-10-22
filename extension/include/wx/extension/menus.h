////////////////////////////////////////////////////////////////////////////////
// Name:      menus.h
// Purpose:   Declaration of wxExMenus class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/xml/xml.h>
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
  static int AddCommands(
    /// node with data
    const wxXmlNode* node,
    /// the commands to be filled
    T& commands) {
    wxXmlNode *child = node->GetChildren();
    while (child)
    {
      if (child->GetName() == "commands")
      {
        AddCommand(child, commands);
      }
      child = child->GetNext();
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
    const wxString unused = "XXXXX";  
    wxString prev_menu = unused;
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
            wxEmptyString,
            it.GetType() & wxExMenuCommand::MENU_COMMAND_ELLIPSES));
        if (it.GetType() & wxExMenuCommand::MENU_COMMAND_SEPARATOR)
        {
          usemenu->AppendSeparator();
        }
      }
      i++;
    }
    return menu->GetMenuItemCount();};
  
  /// Returns the xml filename.
  static const wxFileName GetFileName() {
    return wxFileName(wxExConfigDir(), "menus.xml");};
  
  /// Loads entries from xml document.
  /// Returns false if document could not be loaded, or
  /// no entries were added.
  template <typename T>
  static bool Load(const std::string& name, T& entries) {
    wxXmlDocument doc;
    if (!GetFileName().FileExists() ||
        !doc.Load(GetFileName().GetFullPath()))
    {
      return false;
    }
    entries.clear();
    wxXmlNode* child = doc.GetRoot()->GetChildren();
    while (child)
    {
      if (child->GetName() == name)
      {
        entries.push_back({child});
      }
      child = child->GetNext();
    }
    return !entries.empty();};
private:
  template <typename T>
  static void AddCommand(const wxXmlNode* node, T& commands)
  {
    wxXmlNode* child = node->GetChildren();
    while (child)
    {
      if (child->GetName() == "command")
      {
        const wxString content = child->GetNodeContent().Strip(wxString::both);
        const wxString attrib = child->GetAttribute("type");
        const wxString submenu = child->GetAttribute("submenu");
        const wxString subcommand = child->GetAttribute("subcommand");
        commands.push_back(
          {content.ToStdString(), 
           attrib.ToStdString(), 
           submenu.ToStdString(), 
           subcommand.ToStdString()});
      }
      child = child->GetNext();
    }};
};
