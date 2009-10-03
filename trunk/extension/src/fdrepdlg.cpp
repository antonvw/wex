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

BEGIN_EVENT_TABLE(wxExFindReplaceDialog, wxFindReplaceDialog)
  EVT_BUTTON(wxEVT_COMMAND_FIND_NEXT, wxExFindReplaceDialog::OnCommand)
  EVT_BUTTON(wxEVT_COMMAND_FIND_REPLACE, wxExFindReplaceDialog::OnCommand)
END_EVENT_TABLE()

wxExFindReplaceDialog::wxExFindReplaceDialog(
    wxWindow *parent,
    const wxString& title,
		int style)
  : wxFindReplaceDialog(
      parent, 
      wxExApp::GetConfig()->GetFindReplaceData(), 
      title, 
      style)
{
}

void wxExFindReplaceDialog::OnCommand(wxCommandEvent& event)
{
  event.Skip();

  if (
    event.GetId() == wxID_FIND ||
    event.GetId() == wxID_REPLACE)
  {
    // Match word and regular expression do not work together.
    if (wxExApp::GetConfig()->GetFindReplaceData()->MatchWord())
    {
      wxExApp::GetConfig()->GetFindReplaceData()->SetIsRegularExpression(false);
    }
  }
}
#endif // wxUSE_GUI
