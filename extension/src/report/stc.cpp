/******************************************************************************\
* File:          stc.cpp
* Purpose:       Implementation of class 'wxExSTCWithFrame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/filedlg.h>
#include <wx/extension/util.h>
#include <wx/extension/report/stc.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/util.h>

BEGIN_EVENT_TABLE(wxExSTCWithFrame, wxExSTC)
  EVT_MENU_RANGE(
    ID_EDIT_VCS_LOWEST, 
    ID_EDIT_VCS_HIGHEST, 
    wxExSTCWithFrame::OnCommand)
END_EVENT_TABLE()

wxExSTCWithFrame::wxExSTCWithFrame(wxWindow* parent,
  wxExFrameWithHistory* frame,
  const wxString& value,
  long flags,
  const wxString& title,
  long type,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExSTC(parent, value, flags, title, type, id, pos, size, style)
  , m_Frame(frame)
{
}

wxExSTCWithFrame::wxExSTCWithFrame(wxWindow* parent,
  wxExFrameWithHistory* frame,
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags,
  long type,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExSTC(
      parent, 
      filename, 
      line_number, 
      match, 
      flags, 
      type, 
      id, 
      pos, 
      size, 
      style)
  , m_Frame(frame)
{
  m_Frame->SetRecentFile(GetFileName().GetFullPath());
}

wxExSTCWithFrame::wxExSTCWithFrame(
  const wxExSTC& stc, 
  wxExFrameWithHistory* frame)
  : wxExSTC(stc)
  , m_Frame(frame)
{
}

void wxExSTCWithFrame::OnCommand(wxCommandEvent& command)
{
  if (wxExFileDialog(this, &GetFile()).ShowModalIfChanged() == wxID_CANCEL)
  {
    return;
  }

  if (command.GetId() > ID_EDIT_VCS_LOWEST && 
      command.GetId() < ID_EDIT_VCS_HIGHEST)
  {
    // Cannot move this code to wxExSTC, because of member m_Frame.
    wxArrayString files;
    files.Add(GetFileName().GetFullPath());
    wxExVCSExecute(m_Frame, command.GetId(), files);
  }
  else
  {
    wxFAIL;
  }
}

bool wxExSTCWithFrame::Open(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  bool retValue;

  if (flags & STC_WIN_FROM_OTHER)
  {
    retValue = m_Frame->OpenFile(filename, line_number, match, flags);
  }
  else
  {
    retValue = wxExSTC::Open(filename, line_number, match, flags);

    if (retValue)
    {
      m_Frame->SetRecentFile(filename.GetFullPath());
    }
  }

  return retValue;
}

void wxExSTCWithFrame::PropertiesMessage(long flags)
{
  wxExSTC::PropertiesMessage(flags);
  
  const wxString file = GetName() + 
    (GetReadOnly() ? " [" + _("Readonly") + "]": wxString(wxEmptyString));
    
  m_Frame->SetTitle(file);
}
