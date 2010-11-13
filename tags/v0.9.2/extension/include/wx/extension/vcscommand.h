////////////////////////////////////////////////////////////////////////////////
// Name:      vcscommand.h
// Purpose:   Declaration of wxExVCSCommand class
// Author:    Anton van Wezenbeek
// Created:   2010-08-27
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
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
    VCS_COMMAND_IS_BOTH,
    VCS_COMMAND_IS_POPUP,
    VCS_COMMAND_IS_MAIN,
    VCS_COMMAND_IS_UNKNOWN,
  };
  
  /// Default constructor.
  wxExVCSCommand();

  /// Constructor, specify command and type.
  /// The submenu member is set to specified submenu if not empty,
  /// otherwise to specified subcommand.
  /// Increments the no.
  wxExVCSCommand(
    const wxString& command,
    const wxString& type = wxEmptyString,
    const wxString& submenu = wxEmptyString,
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
  int GetType() const {return m_Type;};
  
  /// Returns true if this is a add like command.
  bool IsAdd() const;

  /// Returns true if this is a checkout like command.
  bool IsCheckout() const;

  /// Returns true if this is a commit like command.
  bool IsCommit() const;

  /// Returns true if this is a diff like command.
  bool IsDiff() const;

  /// Returns true if this is a help like command.
  bool IsHelp() const;

  /// Returns true if this command can behave like
  /// opening a file.
  bool IsOpen() const;

  /// Returns true if this is a update like command.
  bool IsUpdate() const;

  /// Resets the number of instances, so the no
  /// will start from 0 again.
  static void ResetInstances() {m_Instances = 0;};
private:
  int From(const wxString& type) const;

  static int m_Instances;
  
  wxString m_Command;
  wxString m_SubMenu;

  bool m_SubMenuIsCommand;
  int m_No;
  int m_Type;
};
#endif
