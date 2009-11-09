/******************************************************************************\
* File:          svn.h
* Purpose:       Declaration of wxExSVN class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXSVN_H
#define _EXSVN_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/filename.h>

/// SVN types supported.
enum wxExSVNType
{
  SVN_NONE,   ///< not ok value
  SVN_ADD,    ///< svn add
  SVN_BLAME,  ///< svn blame
  SVN_CAT,    ///< svn cat
  SVN_COMMIT, ///< svn commit
  SVN_DIFF,   ///< svn diff
  SVN_HELP,   ///< svn help
  SVN_INFO,   ///< svn info
  SVN_LOG,    ///< svn log
  SVN_LS,     ///< svn ls
  SVN_REVERT, ///< svn revert
  SVN_STAT,   ///< svn stat
  SVN_UPDATE, ///< svn update
};

#if wxUSE_GUI

class wxExSTCEntryDialog;

/// This class collects all svn handling.
class wxExSVN
{
public:
  /// Default constructor.
  wxExSVN();

  /// Constructor, specify the command type and a fullpath.
  wxExSVN(wxExSVNType command, const wxString& fullpath = wxEmptyString);

  /// Constructor, specify the command id and a fullpath.
  wxExSVN(int command_id, const wxString& fullpath = wxEmptyString);

  /// Shows a dialog with options, returns dialog return code.
  int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Set SVN And Comparator")) const;

  /// Returns true if specified filename (a path) is a svn directory.
  bool DirExists(const wxFileName& filename) const;

  /// Execute the svn command, and collects the output.
  /// If no fullpath was specified, a dialog with base folder is shown, otherwise
  /// the specified fullpath is used for getting svn contents from.
  /// If you use parent NULL, then the dialog is not shown,
  /// and defaults from the config are used.
  /// Returns wxID_CANCEL if dialog was cancelled, wxID_OK if okay, or wxID_ABORT if errors
  /// were reported by svn otherwise.
  wxStandardID Execute(wxWindow* parent);

  /// Execute and if not cancelled shows output in a dialog.
  /// Returns return code from execute.
  wxStandardID ExecuteAndShowOutput(wxWindow* parent);

  /// Returns the svn object.
  static wxExSVN* Get(bool createOnDemand = true);

  /// Gets the flags and command (without the 'svn') used to get the output.
  const wxString& GetCommandWithFlags() const {return m_CommandWithFlags;};

  /// Gets the output from Execute.
  const wxString& GetOutput() const {return m_Output;};

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object (both the parameter and returned value may be NULL). 
  static wxExSVN* Set(wxExSVN* svn);

  /// Shows output from Execute in a dialog only.
  void ShowOutput(wxWindow* parent) const;

  /// Returns true if SVN usage is set in the config.
  bool Use() const;
private:
  wxExSVNType GetType(int command_id) const;
  void Initialize();
  bool UseFlags() const;
  bool UseSubcommand() const;

  const wxExSVNType m_Type;

  wxString m_Caption;
  wxString m_Command;
  wxString m_CommandWithFlags;
  wxString m_Output;

  const wxString m_FullPath;

  wxStandardID m_ReturnCode;
  static wxExSVN* m_Self;
  static wxExSTCEntryDialog* m_STCEntryDialog;
  static wxString m_UsageKey;
};
#endif
#endif
