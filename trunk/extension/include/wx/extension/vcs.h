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
#include <wx/filename.h>
#include <wx/extension/command.h>
#include <wx/extension/vcscommand.h>
#include <wx/extension/vcsentry.h>

/// This class collects all vcs handling.
/// The VCS entries are read in from vcs.xml, this is done
/// during wxExApp startup.
/// This class is derived from wxExCommand, allowing 
/// to execute a command, and offering basic for
/// showing output. It also has a vcs commmand,
/// that contains info about the current vcs command to be (or is)
/// executed.
class WXDLLIMPEXP_BASE wxExVCS : public wxExCommand
{
public:
  // The vcs id's here can be set using the config dialog, and are not
  // present in the vcs.xml. These enums should be public,
  // as new entries should start after the last one here.
  enum
  {
    VCS_NONE = 0, // no version control
    VCS_AUTO, // uses the VCS appropriate for current file
  };

  /// Default constructor, specify several files and the menu command id.
  /// If the files array is empty, ShowDialog will show
  /// a combobox for selecting a vcs folder.
  /// The menu command id will be used to set the vcs command.
  wxExVCS(
    const wxArrayString& files = wxArrayString(),
    int menu_id = -1);
  
  /// Constructor for vcs from specified filename.
  /// This must be an existing xml file containing all vcs.
  wxExVCS(const wxFileName& filename) {m_FileName = filename;};

#if wxUSE_GUI
  /// Shows a dialog with options, returns dialog return code.
  int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Set VCS")) const;
#endif    

  /// Returns true if specified filename (a path) is a vcs directory.
  static bool DirExists(const wxFileName& filename);

  /// Executes the vcs command, and collects the output.
  long Execute();

#if wxUSE_GUI
  /// Shows a dialog and executes the vcs command if not cancelled.
  /// If no fullpath was specified, a dialog with base folder is shown, 
  /// otherwise the specified fullpath is used for getting vcs contents from.
  /// Returns wxID_CANCEL if dialog was cancelled, an execute error occurred, 
  /// or there is no output collected. Returns wxID_OK if okay (use GetError
  /// to check whether the output contains errors or normal info).
  wxStandardID ExecuteDialog(wxWindow* parent);
#endif    

  /// Gets the current vcs command.  
  const wxExVCSCommand& GetCommand() const {return m_Command;};

  /// Gets the current vcs entry.
  const wxExVCSEntry& GetEntry() const {return m_Entry;};
  
  /// Gets the xml filename.
  static const wxFileName& GetFileName() {return m_FileName;};
  
  /// Gets the flags used to run the command.
  const wxString GetFlags() const;

  /// Reads all vcs (first clears them) from file.
  /// Returns true if the file could be read and loaded as valid xml file.
  static bool Read();

#if wxUSE_GUI
  /// Combines all in one method. Calls the ExecuteDialog,
  /// and calls ShowOutput if return code was wxID_OK.
  /// Returns return code from ExecuteDialog.
  wxStandardID Request(wxWindow* parent);
#endif  

#if wxUSE_GUI
  /// Overriden from base class.
  virtual void ShowOutput(const wxString& caption = wxEmptyString) const;
#endif

  /// Returns true if VCS usage is set in the config.
  bool Use() const;
private:
  static bool CheckPath(const wxString& vcs, const wxFileName& fn);
  static bool CheckPathAll(const wxString& vcs, const wxFileName& fn);
  static const wxExVCSEntry FindEntry(const wxFileName& filename);
  const wxString GetFile() const;
  void Initialize(int command_id);
  int ShowDialog(wxWindow* parent);
  bool UseFlags() const {
    return !m_Command.IsHelp();};
  bool UseSubcommand() const {
    return m_Command.IsHelp();};
  
  wxExVCSCommand m_Command;
  wxExVCSEntry m_Entry;

  wxArrayString m_Files;
  wxString m_Caption;
  wxString m_FlagsKey;

  static std::map<wxString, wxExVCSEntry> m_Entries;
  static wxFileName m_FileName;
};
#endif
