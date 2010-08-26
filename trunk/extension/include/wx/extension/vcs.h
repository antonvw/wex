/******************************************************************************\
* File:          vcs.h
* Purpose:       Declaration of wxExVCS class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXVCS_H
#define _EXVCS_H

#include <map>
#include <vector>
#include <wx/xml/xml.h>
#include <wx/extension/filename.h>

class wxMenu;
class wxExSTCEntryDialog;

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
  void BuildMenu(int base_id, wxMenu* menu, bool is_popup = true);
#endif
  
  /// Gets the command.
  const wxString GetCommand(int command_id) const;

  /// Gets the name.
  const wxString& GetName() const {return m_Name;};

  /// Gets the no.
  int GetNo() const {return m_No;};
private:
  const std::vector<wxString> ParseNodeCommands(
    const wxXmlNode* node) const;

  static int m_Instances;

  wxString m_Name;
  int m_No;

  std::vector<wxString> m_Commands;
};

/// This class collects all vcs handling.
class wxExVCS
{
public:
  /// Default constructor.
  wxExVCS();

  /// Constructor, specify the menu command id and a filename.
  wxExVCS(int command_id, const wxExFileName& filename = wxExFileName());
  
#if wxUSE_GUI
  /// Builds a menu, default assumes it is a popup menu.
  void BuildMenu(int base_id, wxMenu* menu, bool is_popup = true);
#endif

#if wxUSE_GUI
  /// Shows a dialog with options, returns dialog return code.
  int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Set VCS And Comparator")) const;
#endif    

  /// Returns true if specified filename (a path) is a vcs directory.
  bool DirExists(const wxFileName& filename) const;

  /// Executes the vcs command, and collects the output.
  long Execute();

#if wxUSE_GUI
  /// Shows a dialog and executes the vcs command if not cancelled.
  /// If no fullpath was specified, a dialog with base folder is shown, 
  /// otherwise the specified fullpath is used for getting vcs contents from.
  /// Returns wxID_CANCEL if dialog was cancelled, wxID_OK if okay, 
  /// or wxID_ABORT if errors were reported by vcs otherwise.
  wxStandardID ExecuteDialog(wxWindow* parent);
#endif    

  /// Returns the vcs object.
  static wxExVCS* Get(bool createOnDemand = true);

  /// Gets the flags and command (without the 'vcs') used to get the output.
  const wxString& GetCommandWithFlags() const {return m_CommandWithFlags;};

  /// Gets the output from Execute.
  const wxString& GetOutput() const {return m_Output;};

  /// Returns true if this command can behave like
  /// opening a file.  
  bool IsOpenCommand() const;

#if wxUSE_GUI
  /// Combines all in one method. Calls the ExecuteDialog,
  /// executes if not cancelled, and calls ShowOutput.
  /// Returns return code from execute.
  wxStandardID Request(wxWindow* parent);
#endif  

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object 
  /// (both the parameter and returned value may be NULL). 
  static wxExVCS* Set(wxExVCS* vcs);

#if wxUSE_GUI
  /// Shows output from Execute in a dialog.
  void ShowOutput(wxWindow* parent) const;
#endif  

  /// Does current vcs allow keyword expansion.
  bool SupportKeywordExpansion() const;

  /// Returns true if VCS usage is set in the config.
  bool Use() const;
private:
  bool CheckPath(const wxString& vcs, const wxFileName& fn) const;
  bool CheckPathAll(const wxString& vcs, const wxFileName& fn) const;
  const wxString GetName() const;
  long GetNo(const wxString& name) const;
  int GetType(int command_id) const;
  void Initialize();
  bool Read();
  int ShowDialog(wxWindow* parent);
  long Use(const wxFileName& filename) const;
  bool UseFlags() const;
  bool UseSubcommand() const;

  const int m_Command;
  const wxExFileName m_FileName;

  std::map<wxString, wxExVCSEntry> m_Entries;

  wxString m_Caption;
  wxString m_CommandString;
  wxString m_CommandWithFlags;
  wxString m_FlagsKey;
  wxString m_Output;

  static wxExVCS* m_Self;
#if wxUSE_GUI
  static wxExSTCEntryDialog* m_STCEntryDialog;
#endif  
};
#endif
