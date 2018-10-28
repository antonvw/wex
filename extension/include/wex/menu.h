////////////////////////////////////////////////////////////////////////////////
// Name:      menu.h
// Purpose:   Declaration of wex::menu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h> // for wxArtID
#include <wx/menu.h>
#include <wex/path.h>

namespace wex
{
  /// Adds artid, edit, printing and tool menu items to wxMenu.
  class menu : public wxMenu
  {
  public:
    /// The menu styles.
    enum
    {
      IS_READ_ONLY = 0x0001, ///< readonly control
      IS_SELECTED  = 0x0002, ///< text is selected somewhere on the control
      IS_EMPTY     = 0x0004, ///< control is empty

      ALLOW_CLEAR  = 0x0008, ///< add clear item in menu
      CAN_PASTE    = 0x0010, ///< add paste item in menu

      DEFAULT      = CAN_PASTE  ///< default
    };

    /// Default constructor.
    menu(long style = DEFAULT);
    
    /// Construct a menu with a title.
    menu(const std::string& title, long style = 0);

    /// Appends a menu item for stock menu id's
    /// using automatic naming, help text and art id.
    /// Appends a menu item.
    wxMenuItem* Append(
      int id,
      const std::string& name = std::string(),
      const std::string& helptext = std::string(),
      const wxArtID& artid = std::string());

    /// Appends edit menu items, depending on the style 
    /// specified during construction.
    void append_edit(bool add_invert = false);

    /// Appends print menu items.
    void append_print();

    /// Appends a separator.
    /// If previous item was a separator, it ignores this one.
    /// If no items have yet been appended, it ignores this one.
    void append_separator();

    /// Appends a submenu.
    void append_submenu(
      wxMenu *submenu,
      const std::string& text,
      const std::string& help = std::string(),
      int itemid = wxID_ANY);

    /// Appends a tools submenu.
    /// Returns true if items have been appended.
    bool append_tools(int itemid = wxID_ANY);

    /// Appends VCS menu items.
    /// Returns true if items have been appended.
    bool append_vcs(
      /// if a filename is specified the menu is built as a submenu,
      /// otherwise as menu items.
      const path& filename = path(),
      /// shows modal dialog if necessary
      bool show_modal = true);

    /// Returns the style.
    auto style() const {return m_Style;};

    /// Sets the style.
    void style(long style) {m_Style = style;};
  private:
    long m_Style;
  };
};
