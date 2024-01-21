////////////////////////////////////////////////////////////////////////////////
// Name:      menu-command.h
// Purpose:   Declaration of wex::menu_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

#include <bitset>
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
    INCLUDE_ACCELL,         ///< includes accelerator
  };

  /// A typedef containing command flags.
  typedef std::bitset<2> include_t;

  /// The command type flags as read from xml file.
  enum
  {
    IS_LINES = 0, ///< command supports lines
    IS_ASKED,     ///< command is followed by an ellipses if asked
    IS_POPUP,     ///< command in popup menu
    IS_MAIN,      ///< command in main menu
    IS_SELECTED,  ///< command only shown if text selected
    IS_VISUAL,    ///< command only shown if visual mode
    SEPARATOR,    ///< command is followed by a separator
    ELLIPSES,     ///< command is followed by an ellipses
  };

  /// A typedef containing type flags.
  typedef std::bitset<8> type_t;

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
  const std::string& control() const { return m_control; }

  /// Returns the flags.
  const std::string& flags() const { return m_flags; }

  /// Returns the command (and subcommand and accelerators if necessary).
  const std::string
  get_command(include_t type = include_t().set(INCLUDE_SUBCOMMAND)) const;

  /// Returns the submenu.
  const std::string& submenu() const { return m_submenu; }

  /// Returns true if this is a help like command.
  bool is_help() const { return get_command(0) == "help"; }

  /// Returns the type.
  const type_t& type() const { return m_type; }

  /// Returns the menu text.
  const std::string& text() const { return m_text; }

  /// Returns true if a subcommand can be used for this command.
  bool use_subcommand() const;

private:
  std::string m_command, m_control, m_flags, m_submenu, m_text;

  bool   m_submenu_is_command{false};
  type_t m_type{0};
};
}; // namespace wex
