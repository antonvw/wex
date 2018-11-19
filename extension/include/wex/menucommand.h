////////////////////////////////////////////////////////////////////////////////
// Name:      menu_command.h
// Purpose:   Declaration of wex::menu_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////
 
#pragma once

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

    /// Constructor.
    menu_command(
      /// Specify command. This is the command as
      /// it will appear on a menu, and might contain accelerators.
      const std::string& command,
      /// The type:
      /// - main: command on main menu
      /// - popup: command on popup menu
      /// - (empty): command on main and popup menu
      /// - separator: separator after this command
      /// - ellipses: ellipses after this command
      const std::string& type = std::string(),
      /// The submenu member is set to specified submenu if not empty,
      /// otherwise to specified subcommand.
      const std::string& submenu = std::string(),
      /// The subcommand (used as submenu, but also used for executing).
      const std::string& subcommand = std::string(),
      /// The flags.
      const std::string& flags = std::string());
    
    /// Returns true if flags are requested for this command.
    /// All commands, except help, and if the flags are present
    /// in menus.xml, support flags.
    bool ask_flags() const {return 
      !is_help() && m_Flags.empty() && m_Flags != "none";};
      
    /// Returns the flags.
    const auto& flags() const {return m_Flags;};

    /// Returns the command (and subcommand and accelerators if necessary).
    const std::string get_command(
      include_t type = include_t().set(INCLUDE_SUBCOMMAND)) const;

    /// Returns the submenu.
    const auto& get_submenu() const {return m_SubMenu;};

    /// Returns true if this is a help like command.
    bool is_help() const {return get_command(0) == "help";};

    /// Returns the type.
    auto & type() const {return m_Type;};
    
    /// Returns true if a subcommand can be used for this command.
    bool use_subcommand() const;
  private:
    std::string m_Command, m_Flags, m_SubMenu;

    bool m_SubMenuIsCommand {false};
    type_t m_Type {0};
  };
};
