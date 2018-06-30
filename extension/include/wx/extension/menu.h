////////////////////////////////////////////////////////////////////////////////
// Name:      menu.h
// Purpose:   Declaration of wxExMenu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h> // for wxArtID
#include <wx/menu.h>
#include <wx/extension/path.h>

/// Adds artid, edit, printing and tool menu items to wxMenu.
class WXDLLIMPEXP_BASE wxExMenu : public wxMenu
{
public:
  /// The menu styles.
  enum
  {
    MENU_IS_READ_ONLY = 0x0001, ///< readonly control
    MENU_IS_SELECTED  = 0x0002, ///< text is selected somewhere on the control
    MENU_IS_EMPTY     = 0x0004, ///< control is empty

    MENU_ALLOW_CLEAR  = 0x0008, ///< add clear item in menu
    MENU_CAN_PASTE    = 0x0010, ///< add paste item in menu

    MENU_DEFAULT      = MENU_CAN_PASTE  ///< default
  };

  /// Default constructor.
  wxExMenu(long style = MENU_DEFAULT);
  
  /// Construct a menu with a title.
  wxExMenu(const std::string& title, long style = 0);

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
  void AppendEdit(bool add_invert = false);

  /// Appends print menu items.
  void AppendPrint();

  /// Appends a separator.
  /// If previous item was a separator, it ignores this one.
  /// If no items have yet been appended, it ignores this one.
  void AppendSeparator();

  /// Appends a submenu.
  void AppendSubMenu(
    wxMenu *submenu,
    const std::string& text,
    const std::string& help = std::string(),
    int itemid = wxID_ANY);

  /// Appends a tools submenu.
  /// Returns true if items have been appended.
  bool AppendTools(int itemid = wxID_ANY);

  /// Appends VCS menu items.
  /// Returns true if items have been appended.
  bool AppendVCS(
    /// if a filename is specified the menu is built as a submenu,
    /// otherwise as menu items.
    const wxExPath& filename = wxExPath(),
    /// shows modal dialog if necessary
    bool show_modal = true);

  /// Returns the style.
  auto GetStyle() const {return m_Style;};

  /// Sets the style.
  void SetStyle(long style) {m_Style = style;};
private:
  long m_Style;
};
