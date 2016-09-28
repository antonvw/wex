////////////////////////////////////////////////////////////////////////////////
// Name:      vcscommand.h
// Purpose:   Declaration of wxExVCSCommand class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/dlimpexp.h>

/// This class contains a single vcs command.
class WXDLLIMPEXP_BASE wxExVCSCommand
{
public:
  enum
  {
    VCS_COMMAND_IS_NONE     = 0x0000, ///< command invalid (from default constructor)
    VCS_COMMAND_IS_BOTH     = 0x0001, ///< command in main and popup menu 
    VCS_COMMAND_IS_POPUP    = 0x0002, ///< command in popup menu 
    VCS_COMMAND_IS_MAIN     = 0x0004, ///< command in main menu 
    VCS_COMMAND_SEPARATOR   = 0x0010  ///< command is followed by a separator
  };
  
  /// Default constructor.
  wxExVCSCommand();

  /// Constructor.
  wxExVCSCommand(
    /// Specify command.
    const std::string& command,
    /// The type (main, popup, both).
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
  
  /// Returns true if this is a add like command.
  bool IsAdd() const;
  
  /// Returns true if this is a blame like command.
  bool IsBlame() const;

  /// Returns true if this is a checkout like command.
  bool IsCheckout() const;

  /// Returns true if this is a commit like command.
  bool IsCommit() const;

  /// Returns true if this is a diff like command.
  bool IsDiff() const;

  /// Returns true if this is a help like command.
  bool IsHelp() const;

  /// Returns true if this is a history like command.
  bool IsHistory() const;
  
  /// Returns true if this command can behave like
  /// opening a file.
  bool IsOpen() const;

  /// Returns true if this is a update like command.
  bool IsUpdate() const;

  /// Returns true if flags can be used for this command.
  bool UseFlags() const;
    
  /// Returns true if a subcommand can be used for this command.
  bool UseSubcommand() const;
private:
  long From(const std::string& type) const;

  std::string m_Command;
  std::string m_SubMenu;

  bool m_SubMenuIsCommand;
  long m_Type;
};
