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
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>

#if wxUSE_GUI

wxExFindReplaceDialog::wxExFindReplaceDialog()
{
}

void wxExFindReplaceDialog::FindDialog(wxWindow* parent, const wxString& title)
{
  Destroy();

  Create(
    parent,
    wxExApp::GetConfig()->GetFindReplaceData(),
    title);

  Show();
}

bool wxExFindReplaceDialog::FindResult(
  const wxString& text, 
  bool find_next, 
  bool& recursive)
{
  if (!recursive)
  {
    recursive = true;
    const wxString where = (find_next) ? _("bottom"): _("top");
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(
      _("Searching for") + " " + wxExQuoted(wxExSkipWhiteSpace(text)) + " " + _("hit") + " " + where);
#endif
    return true;
  }
  else
  {
    recursive = false;
    wxBell();
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(wxExQuoted(wxExSkipWhiteSpace(text)) + " " + _("not found"));
#endif
    return false;
  }
}

void wxExFindReplaceDialog::ReplaceDialog(wxWindow* parent, const wxString& title)
{
  Destroy();

  Create(
    parent,
    wxExApp::GetConfig()->GetFindReplaceData(),
    title,
    wxFR_REPLACEDIALOG);

  Show();
}

#endif // wxUSE_GUI
