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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/filename.h>

class wxExSTCEntryDialog;

/// This class collects all vcs handling.
class wxExVCS
{
public:
  /// VCS types supported.
  enum wxExVCSCommand
  {
    VCS_NO_COMMAND, ///< not ok value
    VCS_ADD,      ///< vcs add
    VCS_BLAME,    ///< vcs blame
    VCS_CAT,      ///< vcs cat
    VCS_COMMIT,   ///< vcs commit
    VCS_DIFF,     ///< vcs diff
    VCS_HELP,     ///< vcs help
    VCS_INFO,     ///< vcs info
    VCS_LOG,      ///< vcs log
    VCS_LS,       ///< vcs ls
    VCS_PROPLIST, ///< vcs prop list
    VCS_PROPSET,  ///< vcs prop set
    VCS_REVERT,   ///< vcs revert
    VCS_STAT,     ///< vcs stat
    VCS_UPDATE,   ///< vcs update
  };

  enum wxExVCSSystem
  {
    VCS_NONE, ///< no version control
    VCS_GIT,  ///< GIT version control
    VCS_SVN,  ///< Subversion version control
  };

  /// Default constructor.
  wxExVCS();

  /// Constructor, specify the command and a fullpath.
  wxExVCS(wxExVCSCommand command, const wxString& fullpath = wxEmptyString);

  /// Constructor, specify the command id and a fullpath.
  wxExVCS(int command_id, const wxString& fullpath = wxEmptyString);

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

  /// Gets the command string (without the 'vcs') used to get the output.
  const wxString& GetCommand() const {return m_CommandString;};

  /// Gets the flags and command (without the 'vcs') used to get the output.
  const wxString& GetCommandWithFlags() const {return m_CommandWithFlags;};

  /// Gets the output from Execute.
  const wxString& GetOutput() const {return m_Output;};

#if wxUSE_GUI
  /// Combines all in one method. Shows the dialog,
  /// executes if not cancelled, and shows output in the STC dialog.
  /// Returns return code from execute.
  wxStandardID Request(wxWindow* parent);
#endif  

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object 
  /// (both the parameter and returned value may be NULL). 
  static wxExVCS* Set(wxExVCS* vcs);

#if wxUSE_GUI
  /// Shows output from Execute in a dialog only.
  void ShowOutput(wxWindow* parent) const;
#endif  

  /// Returns true if VCS usage is set in the config.
  bool Use() const;
private:
  wxExVCSCommand GetType(int command_id) const;
  void Initialize();
  int ShowDialog(wxWindow* parent);
  bool UseFlags() const;
  bool UseSubcommand() const;

  const wxExVCSCommand m_Command;

  wxString m_Caption;
  wxString m_CommandString;
  wxString m_CommandWithFlags;
  wxString m_Output;

  const wxString m_FullPath;
  wxString m_FlagsKey;

  static wxExVCS* m_Self;
#if wxUSE_GUI
  static wxExSTCEntryDialog* m_STCEntryDialog;
#endif  
};
#endif
