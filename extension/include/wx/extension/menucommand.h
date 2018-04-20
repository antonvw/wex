////////////////////////////////////////////////////////////////////////////////
// Name:      menucommand.h
// Purpose:   Declaration of wxExMenuCommand class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include <string>
#include <wx/dlimpexp.h>

/// The command as returned by GetCommand.
enum
{
  COMMAND_INCLUDE_NONE        = 0x0000, ///< no other commands
  COMMAND_INCLUDE_SUBCOMMAND  = 0x0001, ///< includes a possible subcommand
  COMMAND_INCLUDE_ACCELL      = 0x0002, ///< includes accelerator
};

/// The command type as read from xml file.  
enum
{
  MENU_COMMAND_IS_NONE   = 0x0000, ///< command invalid (from default constructor)
  MENU_COMMAND_IS_POPUP  = 0x0001, ///< command in popup menu 
  MENU_COMMAND_IS_MAIN   = 0x0002, ///< command in main menu 
  MENU_COMMAND_SEPARATOR = 0x0004, ///< command is followed by a separator
  MENU_COMMAND_ELLIPSES  = 0x0008, ///< command is followed by an ellipses
};

/// This class contains a single menu command.
/// The menu command is meant to be used as command as
/// e.g. vcs or debug command directly.
class WXDLLIMPEXP_BASE wxExMenuCommand
{
public:
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
    const std::string& subcommand = std::string(),
    /// The flags.
    const std::string& flags = std::string());
  
  /// Returns true if flags are requested for this command.
  /// All commands, except help, and if the flags are present
  /// in menus.xml, support flags.
  bool AskFlags() const {return 
    !IsHelp() && m_Flags.empty() && m_Flags != "none";};
    
  /// Returns the command (and subcommand and accelerators if necessary).
  const std::string GetCommand(
    long type = COMMAND_INCLUDE_SUBCOMMAND) const;

  /// Returns the flags.
  const auto& GetFlags() const {return m_Flags;};

  /// Returns the submenu.
  const auto& GetSubMenu() const {return m_SubMenu;};

  /// Returns the type.
  auto GetType() const {return m_Type;};
  
  /// Returns true if this is a help like command.
  bool IsHelp() const {return GetCommand(COMMAND_INCLUDE_NONE) == "help";};

  /// Returns true if a subcommand can be used for this command.
  bool UseSubcommand() const;
private:
  std::string m_Command, m_Flags, m_SubMenu;

  bool m_SubMenuIsCommand {false};
  long m_Type {MENU_COMMAND_IS_NONE};
};
