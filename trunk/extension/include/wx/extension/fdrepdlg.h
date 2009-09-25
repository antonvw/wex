/******************************************************************************\
* File:          fdrepdlg.h
* Purpose:       Declaration of wxExFindReplaceDialog class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXFDREPDLG_H
#define _EXFDREPDLG_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/fdrepdlg.h> // for wxFindReplaceDialog

// Only if we have a gui.
#if wxUSE_GUI

/// Offers a general find interface.
class wxExFindReplaceDialog : public wxFindReplaceDialog
{
public:
  /// Default constructor.
  wxExFindReplaceDialog();

  /// Shows a find dialog.
  void FindDialog(
    wxWindow* parent, 
    const wxString& title = _("Find"));

  /// Shows searching for in the statusbar, and calls FindNext.
  bool FindResult(const wxString& text, bool find_next, bool& recursive);

  /// Shows a replace dialog.
  void ReplaceDialog(
    wxWindow* parent, 
    const wxString& title = _("Replace"));
};
#endif // wxUSE_GUI
#endif
