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
#include <wx/xml/xml.h>
#include <wx/extension/filename.h>
#include <wx/extension/vcscommand.h>
#include <wx/extension/vcsentry.h>

class wxMenu;
class wxExSTCEntryDialog;

/// This class collects all vcs handling.
/// The VCS entries are read in from vcss.xml.
class wxExVCS
{
public:
  // The vcs id's here can be set using the config dialog, and are not
  // present in the vcss.xml. These enums should be public,
  // as new entries should start after the last one here.
  enum
  {
    VCS_NONE = 0, // no version control
    VCS_AUTO, // uses the VCS appropriate for current file
  };

  /// Default constructor.
  wxExVCS();

  /// Constructor for vcss from specified filename.
  /// This must be an existing xml file containing all vcss.
  /// It does not Read this file, however if you use the global Get,
  /// it both constructs and reads the vcs.
  wxExVCS(const wxFileName& filename);

  /// Constructor, specify the menu command id and a filename.
  wxExVCS(int menu_id, const wxExFileName& filename = wxExFileName());
  
#if wxUSE_GUI
  /// Builds a menu, default assumes it is a popup menu.
  void BuildMenu(
    int base_id, 
    wxMenu* menu, 
    const wxExFileName& filename = wxExFileName(),
    bool is_popup = true);
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

  /// Gets the xml filename.
  static const wxFileName& GetFileName() {return m_FileNameXML;};

  /// Gets the output from Execute.
  const wxString& GetOutput() const {return m_Output;};

  /// Returns true if this command can behave like
  /// opening a file.  
  bool IsOpenCommand() const {
    return m_Command.IsOpen();};

  /// Reads all containers (first clears them )from file.
  /// Returns true if the file could be read and loaded as valid xml file.
  bool Read();

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
  static bool CheckPath(const wxString& vcs, const wxFileName& fn);
  static bool CheckPathAll(const wxString& vcs, const wxFileName& fn);
  static long FindNo(const wxString& name);
  static const wxString GetName(const wxFileName& filename);
  static long GetNo(const wxFileName& filename);
  void Initialize(int command_id);
  int ShowDialog(wxWindow* parent);
  bool UseFlags() const;
  bool UseSubcommand() const;

  wxExVCSCommand m_Command;

  wxString m_Caption;
  wxString m_CommandWithFlags;
  wxString m_FlagsKey;
  wxString m_Output;

  static std::map<wxString, wxExVCSEntry> m_Entries;
  static wxExFileName m_FileName;
  static wxFileName m_FileNameXML;
  static wxExVCS* m_Self;
#if wxUSE_GUI
  static wxExSTCEntryDialog* m_STCEntryDialog;
#endif  
};
#endif
