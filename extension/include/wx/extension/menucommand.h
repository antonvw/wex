////////////////////////////////////////////////////////////////////////////////
// Name:      menucommand.h
// Purpose:   Declaration of wxExMenuCommand class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include <string>
#include <wx/dlimpexp.h>

/// This class contains a single menu command.
/// The menu command is meant to be used as command as
/// e.g. vcs or debug command directly.
class WXDLLIMPEXP_BASE wxExMenuCommand
{
public:
  enum
  {
    MENU_COMMAND_IS_NONE   = 0x0000, ///< command invalid (from default constructor)
    MENU_COMMAND_IS_POPUP  = 0x0001, ///< command in popup menu 
    MENU_COMMAND_IS_MAIN   = 0x0002, ///< command in main menu 
    MENU_COMMAND_SEPARATOR = 0x0004, ///< command is followed by a separator
    MENU_COMMAND_ELLIPSES  = 0x0008, ///< command is followed by an ellipses
  };
  
  /// Default constructor.
  wxExMenuCommand() {;};

  /// Constructor.
  wxExMenuCommand(
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
    const std::string& subcommand = std::string());
  
  /// Returns the command (and subcommand and accelerators if necessary).
  const std::string GetCommand(
    bool include_subcommand = true,
    bool include_accelerators = false) const;
  
  /// Returns the submenu.
  const auto& GetSubMenu() const {return m_SubMenu;};

  /// Returns the type.
  long GetType() const {return m_Type;};
  
  /// Returns true if this is a help like command.
  bool IsHelp() const {return GetCommand(false) == "help";};

  /// Returns true if flags can be used for this command.
  // All commands, except help support flags.
  bool UseFlags() const {return !IsHelp();};
    
  /// Returns true if a subcommand can be used for this command.
  bool UseSubcommand() const;
private:
  long From(const std::string& type) const;

  std::string m_Command;
  std::string m_SubMenu;

  bool m_SubMenuIsCommand = false;
  long m_Type = MENU_COMMAND_IS_NONE;
};
