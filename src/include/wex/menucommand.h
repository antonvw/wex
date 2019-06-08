////////////////////////////////////////////////////////////////////////////////
// Name:      menu_command.h
// Purpose:   Declaration of wex::menu_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
      INCLUDE_ACCELL     = 1, ///< includes accelerator
    };

    /// The command type flags as read from xml file.  
    enum
    {
      IS_POPUP  = 0, ///< command in popup menu 
      IS_MAIN   = 1, ///< command in main menu 
      SEPARATOR = 2, ///< command is followed by a separator
      ELLIPSES  = 3, ///< command is followed by an ellipses
    };

    typedef std::bitset<2> include_t;
    typedef std::bitset<4> type_t;
    
    /// Default constructor.
    menu_command() {;};

    /// Constructor using xml node.
    menu_command(const pugi::xml_node& node);
    
    /// Returns true if flags are requested for this command.
    /// All commands, except help, and if the flags are present
    /// in menus.xml, support flags.
    bool ask_flags() const {return 
      !is_help() && m_flags.empty() && m_flags != "none";};
      
    /// Returns the control key.
    const auto& control() const {return m_control;};

    /// Returns the flags.
    const auto& flags() const {return m_flags;};

    /// Returns the command (and subcommand and accelerators if necessary).
    const std::string get_command(
      include_t type = include_t().set(INCLUDE_SUBCOMMAND)) const;

    /// Returns the submenu.
    const auto& submenu() const {return m_submenu;};

    /// Returns true if this is a help like command.
    bool is_help() const {return get_command(0) == "help";};

    /// Returns the type.
    auto & type() const {return m_type;};
    
    /// Returns true if a subcommand can be used for this command.
    bool use_subcommand() const;
  private:
    std::string m_control, m_command, m_flags, m_submenu;

    bool m_submenu_is_command {false};
    type_t m_type {0};
  };
};
