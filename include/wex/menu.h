////////////////////////////////////////////////////////////////////////////////
// Name:      menu.h
// Purpose:   Declaration of wex::menu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <wex/menu-item.h>
#include <wx/menu.h>

namespace wex
{
/// Adds artid, edit, printing and tool menu items to wxMenu.
class menu : public wxMenu
{
public:
  /// The menu flags.
  enum
  {
    IS_EMPTY = 0, ///< control is empty
    IS_POPUP,     ///< menu should appear as popup, instead of main menu
    IS_READ_ONLY, ///< readonly control
    IS_SELECTED,  ///< text is selected somewhere on the control
    IS_VISUAL,    ///< the control has visual mode

    ALLOW_CLEAR, ///< add clear item in menu
    CAN_PASTE,   ///< add paste item in menu

    DEFAULT = CAN_PASTE ///< default
  };

  typedef std::bitset<7> menu_t;

  /// Default constructor.
  menu(
    menu_t                        style = menu_t().set(DEFAULT),
    const std::vector<menu_item>& items = {{}});

  /// Constructor with a vector of items.
  menu(const std::vector<menu_item>& items);

  /// Construct a menu with a title.
  menu(const std::string& title, menu_t style);

  /// Appends a vector of items.
  /// Returns number of items appended.
  size_t append(const std::vector<menu_item>& items);

  /// Returns the style.
  auto& style() const { return m_style; }

  /// Sets the style.
  auto& style() { return m_style; }

private:
  /// Appends edit menu items, depending on the style
  /// specified during construction.
  void append_edit(bool add_invert = false);
  void append_print();
  void append_separator();
  void append_tools();

  menu_t m_style;
};
}; // namespace wex
