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

/// SVN types supported.
enum wxExSVNType
{
  SVN_BLAME,  ///< svn blame
  SVN_CAT,    ///< svn cat
  SVN_COMMIT, ///< svn commit
  SVN_DIFF,   ///< svn diff
  SVN_HELP,   ///< svn help
  SVN_INFO,   ///< svn info
  SVN_LOG,    ///< svn log
  SVN_REVERT, ///< svn revert
  SVN_STAT,   ///< svn stat
  SVN_UPDATE, ///< svn update
};

#if wxUSE_GUI

class wxExSTCEntryDialog;

/// This class collects all svn handling.
/// Your application should be derived from wxExApp when using this class.
class wxExSVN
{
public:
  /// Constructor, specify the command type and a fullpath.
  wxExSVN(wxExSVNType command, const wxString& fullpath = wxEmptyString);

  /// Constructor, specify the command id and a fullpath.
  wxExSVN(int command_id, const wxString& fullpath = wxEmptyString);

  /// Execute the svn command, and collects the output.
  /// If no fullpath was specified, a dialog with base folder is shown, otherwise
  /// the specified fullpath is used for getting svn contents from.
  /// If you use parent NULL, then the dialog is not shown,
  /// and defaults from the config are used.
  /// Returns -1 if dialog was cancelled, 0 if okay, or the number of errors
  /// that were reported by svn otherwise.
  int Execute(wxWindow* parent);

  /// Execute and if not cancelled shows output in a dialog.
  /// Returns return code from execute.
  int ExecuteAndShowOutput(wxWindow* parent);

  /// Gets the flags and command (without the 'svn') used to get the output.
  const wxString& GetCommandWithFlags() const {return m_CommandWithFlags;};

  /// Gets the output from Execute.
  const wxString& GetOutput() const {return m_Output;};

  /// Shows output from Execute in a dialog only.
  void ShowOutput(wxWindow* parent) const;
private:
  wxExSVNType GetType(int command_id) const;
  void Initialize();

  const wxExSVNType m_Type;

  wxString m_Caption;
  wxString m_Command;
  wxString m_CommandWithFlags;
  wxString m_Output;

  const wxString m_FullPath;

  int m_ReturnCode;
  static wxExSTCEntryDialog* m_STCEntryDialog;
};
#endif
#endif
