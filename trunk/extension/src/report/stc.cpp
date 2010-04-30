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
#include <wx/config.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/header.h>
#include <wx/extension/vcs.h>
#include <wx/extension/util.h>
#include <wx/extension/report/stc.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/textfile.h>
#include <wx/extension/report/util.h>

BEGIN_EVENT_TABLE(wxExSTCWithFrame, wxExSTCFile)
  EVT_MENU_RANGE(
    ID_EDIT_VCS_LOWEST, 
    ID_EDIT_VCS_HIGHEST, 
    wxExSTCWithFrame::OnCommand)
  EVT_MENU_RANGE(ID_STC_LOWEST, ID_STC_HIGHEST, wxExSTCWithFrame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, wxExSTCWithFrame::OnCommand)
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
  : wxExSTCFile(parent, value, flags, title, type, id, pos, size, style)
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
  : wxExSTCFile(
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
  const wxExSTCFile& stc, 
  wxExFrameWithHistory* frame)
  : wxExSTCFile(stc)
  , m_Frame(frame)
{
}

void wxExSTCWithFrame::BuildPopupMenu(wxExMenu& menu)
{
  wxExSTCFile::BuildPopupMenu(menu);

  if ( GetFileName().FileExists() && GetSelectedText().empty() &&
      (GetMenuFlags() & STC_MENU_COMPARE_OR_VCS))
  {
    if (wxExVCS::Get()->DirExists(GetFileName()))
    {
      menu.AppendVCS();
    }
    else if (!wxConfigBase::Get()->Read(_("Comparator")).empty())
    {
      menu.AppendSeparator();
      menu.Append(ID_STC_COMPARE, wxExEllipsed(_("&Compare Recent Version")));
    }
  }
}

void wxExSTCWithFrame::OnCommand(wxCommandEvent& command)
{
  if (wxExFileDialog(this, this).ShowModalIfChanged() == wxID_CANCEL)
  {
    return;
  }

  if (command.GetId() > ID_TOOL_LOWEST && command.GetId() < ID_TOOL_HIGHEST)
  {
    const wxExTool tool(command.GetId());

    if (wxExTextFileWithListView::SetupTool(tool, m_Frame))
    {
      wxExTextFileWithListView report(GetFileName(), tool);
      report.RunTool();
      tool.Log(&report.GetStatistics().GetElements(), GetFileName().GetFullPath());

      if (tool.IsCount())
      {
        m_Frame->OpenFile(
          tool.GetLogfileName(), 0, wxEmptyString, STC_WIN_FROM_OTHER);
      }
    }
  }
  else if (command.GetId() > ID_EDIT_VCS_LOWEST && 
           command.GetId() < ID_EDIT_VCS_HIGHEST)
  {
    wxExVCSExecute(m_Frame, command.GetId(), GetFileName());
  }
  else switch (command.GetId())
  {
    case ID_STC_ADD_HEADER:
    {
      const wxExHeader header;

      if (header.ShowDialog(this) != wxID_CANCEL)
      {
        if (GetLexer().GetScintillaLexer() == "hypertext")
        {
          GotoLine(1);
        }
        else
        {
          DocumentStart();
        }

        AddText(header.Get(&GetFileName()));
      }
    }
    break;

    case ID_STC_COMPARE:
    {
      wxFileName lastfile;

      if (wxExFindOtherFileName(GetFileName(), NULL, &lastfile))
      {
        wxExCompareFile(GetFileName(), lastfile);
      }
    }
    break;

    default: wxFAIL; break;
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
    retValue = wxExSTCFile::Open(filename, line_number, match, flags);

    if (retValue)
    {
      m_Frame->SetRecentFile(filename.GetFullPath());
    }
  }

  return retValue;
}

void wxExSTCWithFrame::PropertiesMessage()
{
  wxExSTCFile::PropertiesMessage();

  m_Frame->SetTitle(
    GetName() + 
      (GetReadOnly() ? " [" + _("Readonly") + "]": wxString(wxEmptyString)), 
    wxEmptyString);
}
