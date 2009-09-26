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
  /// Constructor.
  wxExFindReplaceDialog(
    wxWindow *parent,
    const wxString& title,
		int style = 0);
};
#endif // wxUSE_GUI
#endif
