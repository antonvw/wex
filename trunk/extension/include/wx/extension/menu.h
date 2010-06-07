/******************************************************************************\
* File:          menu.h
* Purpose:       Declaration of wxExMenu class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXMENU_H
#define _EXMENU_H

#include <wx/artprov.h> // for wxArtID
#include <wx/menu.h>

// Only if we have a gui.
#if wxUSE_GUI

/// Adds artid, edit, printing and tool menu items to wxMenu.
class wxExMenu : public wxMenu
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

    MENU_DEFAULT      = MENU_CAN_PASTE, ///< default
  };

  /// Default constructor.
  wxExMenu(long style = MENU_DEFAULT);

  /// Copy constructor.
  wxExMenu(const wxExMenu& menu);

  /// Appends a menu item for stock menu id's
  /// using automatic naming, help text and art id.
  wxMenuItem* Append(int id);

  /// Appends a menu item.
  wxMenuItem* Append(
    int id,
    const wxString& name,
    const wxString& helptext = wxEmptyString,
    const wxArtID& artid = wxEmptyString);

  /// Appends bar menu items (like status bar etc.).
  void AppendBars();

  /// Appends edit menu items, depending on the style 
  /// specified during construction.
  void AppendEdit(bool add_invert = false);

  /// Appends print menu items.
  void AppendPrint();

  /// Appends a separator.
  /// If previous item was a separator, it ignores this one.
  /// If no items have yet been appended, it ignores this one.
  void AppendSeparator();

  /// Appends a submenu (and resets the is separator member).
  void AppendSubMenu(
    wxMenu *submenu,
    const wxString& text,
    const wxString& help = wxEmptyString,
    int itemid = wxID_ANY);

  /// Appends a tools submenu.
  /// Returns true if items have been appended.
  bool AppendTools(int itemid = wxID_ANY);

  /// Appends VCS menu items.
  void AppendVCS();

  /// Appends specified VCS menu item.
  void AppendVCS(int id);

  /// Builds VCS menu items.
  void BuildVCS(bool fill);

  /// Is the VCS menu build?
  bool IsVCSBuild() const {return m_MenuVCSFilled;};

  /// Gets the style.
  long GetStyle() const {return m_Style;};

  /// Sets the style.
  void SetStyle(long style) {m_Style = style;};
private:
  bool m_MenuVCSFilled;
  long m_Style;
};
#endif // wxUSE_GUI
#endif
