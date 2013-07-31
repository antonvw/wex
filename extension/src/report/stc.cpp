////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wxExSTCWithFrame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
  int col_number,
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
      col_number,
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
  if (command.GetId() > ID_EDIT_VCS_LOWEST && 
      command.GetId() < ID_EDIT_VCS_HIGHEST)
  {
    if (wxExFileDialog(this, &GetFile()).ShowModalIfChanged() == wxID_OK)
    {
      // Cannot move this code to wxExSTC, because of member m_Frame.
      std::vector< wxString > files;
      files.push_back(GetFileName().GetFullPath());
      wxExVCSExecute(m_Frame, command.GetId() - ID_EDIT_VCS_LOWEST - 1, files);
    }
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
  int col_number,
  long flags)
{
  const bool retValue = (flags & STC_WIN_FROM_OTHER ?
     m_Frame->OpenFile(filename, line_number, match, flags):
     wxExSTC::Open(filename, line_number, match, col_number, flags));

  if (retValue)
  {
    m_Frame->SetRecentFile(filename.GetFullPath());
  }
    
  return retValue;
}

void wxExSTCWithFrame::PropertiesMessage(long flags)
{
  wxExSTC::PropertiesMessage(flags);
  
  if (!(flags & STAT_SYNC))
  {
    const wxString file = GetName() + 
      (GetReadOnly() ? " [" + _("Readonly") + "]": wxString(wxEmptyString));
    
    if (file.empty())
    {
      m_Frame->SetTitle(wxTheApp->GetAppName());
    }
    else
    {
      m_Frame->SetTitle(file);
    }
  }
}
