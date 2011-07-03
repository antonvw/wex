////////////////////////////////////////////////////////////////////////////////
// Name:      vcscommand.h
// Purpose:   Declaration of wxExVCSCommand class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVCSCOMMAND_H
#define _EXVCSCOMMAND_H

#include <wx/string.h>

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
    VCS_COMMAND_SEPARATOR   = 0x0010, ///< command is followed by a separator
  };
  
  /// Default constructor.
  wxExVCSCommand();

  /// Constructor.
  wxExVCSCommand(
    /// Specify command.
    const wxString& command,
    /// The no.
    int no,
    /// The type (main, popup, both).
    const wxString& type = wxEmptyString,
    /// The submenu member is set to specified submenu if not empty,
    /// otherwise to specified subcommand.
    const wxString& submenu = wxEmptyString,
    /// The subcommand (used as submenu, but also used for executing).
    const wxString& subcommand = wxEmptyString);

  /// Gets the command (and subcommand and accelerators if necessary).
  const wxString GetCommand(
    bool include_subcommand = true,
    bool include_accelerators = false) const;
  
  /// Gets the no.
  int GetNo() const {return m_No;};

  /// Returns the submenu.
  const wxString& GetSubMenu() const {return m_SubMenu;};

  /// Gets the type.
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
  int From(const wxString& type) const;

  wxString m_Command;
  wxString m_SubMenu;

  bool m_SubMenuIsCommand;
  int m_No;
  long m_Type;
};
#endif
