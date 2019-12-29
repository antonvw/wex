////////////////////////////////////////////////////////////////////////////////
// Name:      menu-item.h
// Purpose:   Declaration of wex::menu_item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h> // for wxArtID
#include <wex/filehistory.h>
#include <wex/path.h>

namespace wex
{
  class managed_frame;
  class menu;

  /// Offers a single menu item.
  class menu_item
  {
  public:
    /// The item types supported.
    enum type_t
    {
      /// a check menu item
      CHECK,

      /// edit menu items
      EDIT,
      
      /// edit invert menu items
      EDIT_INVERT,
      
      /// exit menu item
      EXIT,
      
      /// file_history menu items
      HISTORY,
      
      /// a normal menu item
      MENU,
      
      /// toggle panes from managaed frame menu items
      PANES,
      
      /// print menu items
      PRINT,
      
      /// a radio menu item
      RADIO,

      /// a separator menu item
      /// If previous item was a separator, it ignores this one.
      /// If no items have yet been appended, it ignores this one.
      SEPARATOR,
      
      /// a submenu item
      SUBMENU,
      
      /// tools menu items
      TOOLS,

      /// vcs menu items
      VCS,
    };

    /// Default constructor, for a SEPARATOR.
    menu_item(
      /// EDIT, EDIT_INVERT, EXIT, PANES, PRINT, TOOLS menu item
      type_t = SEPARATOR);

    /// Constructor for a normal MENU item.
    menu_item(
      /// menu item id
      int id, 
      /// menu name or text
      const std::string& name = std::string(), 
      /// menu help text
      const std::string& help = std::string(),
      /// menu artid
      const wxArtID& art = std::string(),
      /// callback for menu action
      std::function<void(wxCommandEvent&)> action = nullptr,
      /// callback for menu update action
      std::function<void(wxUpdateUIEvent&)> ui = nullptr);
     
    /// Constructor for a checkable item.
    menu_item(
      /// menu item id
      int id, 
      /// menu name or text
      const std::string& name, 
      /// Constructor for a CHECK or RADIO item.
      type_t type,
      /// callback for menu event action
      std::function<void(wxCommandEvent&)> action = nullptr,
      /// callback for menu update action
      std::function<void(wxUpdateUIEvent&)> ui = nullptr,
      /// menu help text
      const std::string& help = std::string());

    /// Constructor for a SUBMENU item.
    menu_item(
      /// menu submenu
      menu* submenu,
      /// menu name or text
      const std::string& name,
      /// menu help text
      const std::string& help = std::string(),
      /// menu item id
      int id = wxID_ANY);

    /// Constructor for a VCS submenu item.
    menu_item(
      /// if a filename is specified the menu is built as a submenu,
      /// otherwise as menu items.
      const path& p,
      /// shows modal dialog if necessary
      bool show_modal = true);
    
    /// Constructor for HISTORY menu item.
    menu_item(
      /// menu item id
      int id, 
      /// object for maintaining / retrieving history
      file_history& history);

    /// Constructor for PANES menu items.
    menu_item(
      /// frame to supply toggled panes
      const managed_frame* frame);

    /// Appends this item(s) to menu.
    void append(wex::menu* menu) const;

    /// Returns menu item id.
    auto id() const {return m_id;};
    
    /// Returns menu item name.
    auto & name() const {return m_name;};
    
    /// Returns menu item type.
    auto type() const {return m_type;};
  private:
    void append_panes(wex::menu* menu) const;
    void append_vcs(wex::menu* menu) const;

    const managed_frame* m_frame {nullptr};
    file_history* m_history {nullptr};
    wex::menu* m_menu {nullptr};
    const bool m_modal {false};
    const wex::path m_path;
    const int m_id {-1};
    const type_t m_type {SEPARATOR};
    const std::string 
      m_help_text,
      m_name;
    const wxArtID m_artid;
  };
};
