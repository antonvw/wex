/******************************************************************************\
* File:          fdrepdlg.cpp
* Purpose:       Implementation of wxExInterface class
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

void wxExInterface::FindDialog(wxWindow* parent, const wxString& caption)
{
  SetFindReplaceData();

  if (m_FindReplaceDialog != NULL)
  {
    m_FindReplaceDialog->Destroy();
  }

  m_FindReplaceDialog = new wxFindReplaceDialog(
    parent,
    wxExApp::GetConfig()->GetFindReplaceData(),
    caption);

  m_FindReplaceDialog->Show();
}

bool wxExInterface::FindResult(
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
    return FindNext(text, find_next);
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

void wxExInterface::OnFindDialog(wxFindDialogEvent& event)
{
  wxExFindReplaceData* frd = wxExApp::GetConfig()->GetFindReplaceData();

  const bool find_next = (frd->GetFlags() & wxFR_DOWN);

  if (event.GetEventType() == wxEVT_COMMAND_FIND_CLOSE)
  {
    m_FindReplaceDialog->Destroy();
    m_FindReplaceDialog = NULL;
  }
  else if (event.GetEventType() == wxEVT_COMMAND_FIND)
  {
    FindNext(frd->GetFindString(), find_next);
  }
  else if (event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)
  {
    FindNext(frd->GetFindString(), find_next);
  }
  else
  {
    wxFAIL;
  }
}

void wxExInterface::ReplaceDialog(wxWindow* parent, const wxString& caption)
{
  if (m_FindReplaceDialog != NULL)
  {
    m_FindReplaceDialog->Destroy();
  }

  m_FindReplaceDialog = new wxFindReplaceDialog(
    parent,
    wxExApp::GetConfig()->GetFindReplaceData(),
    caption,
    wxFR_REPLACEDIALOG);

  m_FindReplaceDialog->Show();
}

#endif // wxUSE_GUI
