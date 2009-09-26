/******************************************************************************\
* File:          fdrepdlg.cpp
* Purpose:       Implementation of wxExFindReplaceDialog class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/fdrepdlg.h>
#include <wx/extension/app.h>
#include <wx/extension/frd.h>

#if wxUSE_GUI

wxExFindReplaceDialog::wxExFindReplaceDialog(
    wxWindow *parent,
    const wxString& title,
		int style)
  : wxFindReplaceDialog(parent, wxExApp::GetConfig()->GetFindReplaceData(), title, style)
{
}
#endif // wxUSE_GUI
