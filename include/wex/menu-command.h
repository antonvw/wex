////////////////////////////////////////////////////////////////////////////////
// Name:      menu-command.h
// Purpose:   Declaration of wex::menu_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <pugixml.hpp>
#include <string>

namespace wex
{
  /// This class contains a single menu command.
  /// The menu command is meant to be used as command as
  /// e.g. vcs or debug command directly.
  class menu_command
  {
  public:
    /// The command flags as used by get_command.
    enum include
    {
      INCLUDE_SUBCOMMAND = 0, ///< includes a possible subcommand
      INCLUDE_ACCELL     = 1, ///< includes accelerator
    };

    typedef std::bitset<2> include_t;

    /// The command type flags as read from xml file.
    enum
    {
      IS_POPUP = 0, ///< command in popup menu
      IS_MAIN,      ///< command in main menu
      IS_SELECTED,  ///< command only shown if text selected
      SEPARATOR,    ///< command is followed by a separator
      ELLIPSES,     ///< command is followed by an ellipses
    };

    typedef std::bitset<5> type_t;

    /// Default constructor using xml node.
    menu_command(const pugi::xml_node& node = pugi::xml_node());

    /// Returns true if flags are requested for this command.
    /// All commands, except help, and if the flags are present
    /// in menus.xml, support flags.
    bool ask_flags() const
    {
      return !is_help() && m_flags.empty() && m_flags != "none";
    };

    /// Returns the control key.
    const auto& control() const { return m_control; };

    /// Returns the flags.
    const auto& flags() const { return m_flags; };

    /// Returns the command (and subcommand and accelerators if necessary).
    const std::string
    get_command(include_t type = include_t().set(INCLUDE_SUBCOMMAND)) const;

    /// Returns the submenu.
    const auto& submenu() const { return m_submenu; };

    /// Returns true if this is a help like command.
    bool is_help() const { return get_command(0) == "help"; };

    /// Returns the type.
    auto& type() const { return m_type; };

    /// Returns the menu text.
    const auto& text() const { return m_text; };

    /// Returns true if a subcommand can be used for this command.
    bool use_subcommand() const;

  private:
    std::string m_command, m_control, m_flags, m_submenu, m_text;

    bool   m_submenu_is_command{false};
    type_t m_type{0};
  };
}; // namespace wex
