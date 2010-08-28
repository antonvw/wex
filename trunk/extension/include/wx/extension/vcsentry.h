/******************************************************************************\
* File:          vcsentry.h
* Purpose:       Declaration of wxExVCSEntry class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXVCSENTRY_H
#define _EXVCSENTRY_H

#include <vector>
#include <wx/xml/xml.h>

class wxMenu;

/// This class contains a single vcs command.
class wxExVCSCommand
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
  wxExVCSCommand()
    : m_Command()
    , m_Type(VCS_COMMAND_IS_UNKNOWN) {;};

  /// Constructor.
  wxExVCSCommand(
    const wxString& command,
    const wxString& type = wxEmptyString)
    : m_Command(command)
    , m_Type(From(type)){;};

  /// Gets the command.
  const wxString GetCommand() const {return m_Command;};

  /// Gets the type.
  int GetType() const {return m_Type;};

  /// Returns true if this command can behave like
  /// opening a file.
  bool IsOpen() const;

  /// Sets the command and type.
  void SetCommand(
    const wxString& command,
    const wxString& type = wxEmptyString) {
    m_Command = command;
    m_Type = From(type);};
private:
  int From(const wxString& type) const;
  wxString m_Command;
  int m_Type;
};

/// This class collects a single vcs.
class wxExVCSEntry
{
public:
  /// Default constructor.
  wxExVCSEntry();
  
  /// Constructor using xml node.
  wxExVCSEntry(const wxXmlNode* node);

#if wxUSE_GUI
  /// Builds a menu, default assumes it is a popup menu.
  void BuildMenu(int base_id, wxMenu* menu, bool is_popup = true) const;
#endif
  
  /// Gets the command.
  const wxString GetCommand(int command_id) const;

  /// Gets the name.
  const wxString& GetName() const {return m_Name;};

  /// Gets the no.
  long GetNo() const {return m_No;};
private:
  const std::vector<wxExVCSCommand> ParseNodeCommands(
    const wxXmlNode* node) const;

  static int m_Instances;

  const wxString m_Name;
  const long m_No;

  std::vector<wxExVCSCommand> m_Commands;
};
#endif
