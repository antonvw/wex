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

BEGIN_EVENT_TABLE(wxExSTCWithFrame, wxExSTC)
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
  long style,
  const wxString& name)
  : wxExSTC(parent, value, flags, title, type, id, pos, size, style, name)
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
  long style,
  const wxString& name)
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
      style, 
      name)
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

void wxExSTCWithFrame::BuildPopupMenu(wxExMenu& menu)
{
  wxExSTC::BuildPopupMenu(menu);

  // Add tools if we have at least some text, the tool flag,
  // and a lexer.
  if (GetSelectedText().empty() && GetTextLength() > 0 &&
     (GetMenuFlags() & STC_MENU_TOOL) &&
      !GetFileName().GetLexer().GetScintillaLexer().empty())
  {
    menu.AppendSeparator();
    menu.AppendTools();
  }

  if (GetMenuFlags() & (STC_MENU_REPORT_FIND | STC_MENU_REPORT_REPLACE))
  {
    menu.AppendSeparator();

    if (GetMenuFlags() & STC_MENU_REPORT_FIND)
    {
      menu.Append(ID_STC_FIND_IN_FILES, wxExEllipsed(_("Find &In Files")));
    }

    if (GetMenuFlags() & STC_MENU_REPORT_REPLACE)
    {
      menu.Append(ID_STC_REPLACE_IN_FILES, wxExEllipsed(_("&Replace In Files")));
    }
  }

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

  if (!GetReadOnly())
  {
    menu.AppendSeparator();
    menu.Append(ID_STC_ADD_HEADER, wxExEllipsed(_("&Add Header")));
  }
}

void wxExSTCWithFrame::OnCommand(wxCommandEvent& command)
{
  wxExFileDialog dlg(this, this);

  if (dlg.ShowModalIfChanged() == wxID_CANCEL) 
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
          tool.GetLogfileName(), 0, wxEmptyString, STC_OPEN_FROM_OTHER);
      }
    }

    return;
  }

  if (command.GetId() > ID_EDIT_VCS_LOWEST && 
      command.GetId() < ID_EDIT_VCS_HIGHEST)
  {
    wxExVCS vcs(command.GetId(), GetFileName().GetFullPath());

    if (command.GetId() == ID_EDIT_VCS_CAT ||
        command.GetId() == ID_EDIT_VCS_BLAME)
    {
      if (vcs.ExecuteDialog(this) == wxID_OK)
      {
        m_Frame->OpenFile(
          GetFileName(), 
          vcs.GetCommandWithFlags(), 
          vcs.GetOutput(),
          wxExSTC::STC_OPEN_READ_ONLY);
      }
    }
    else
    {
      vcs.Request(this);
    }

    return;
  }

  switch (command.GetId())
  {
  case ID_STC_ADD_HEADER:
    {
      const wxExHeader header;

      if (header.ShowDialog(this) != wxID_CANCEL)
      {
        if (GetFileName().GetLexer().GetScintillaLexer() == "hypertext")
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

  case ID_STC_FIND_IN_FILES:
  case ID_STC_REPLACE_IN_FILES:
    m_Frame->FindInFilesDialog(command.GetId());
    break;

  default: wxFAIL;
    break;
  }
}

bool wxExSTCWithFrame::Open(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  bool retValue;

  if (flags & STC_OPEN_FROM_OTHER)
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

void wxExSTCWithFrame::PropertiesMessage() const
{
  wxExSTC::PropertiesMessage();

  const wxString title = 
    GetFileName().FileExists() ? GetFileName().GetFullPath(): GetTitle();

  m_Frame->SetTitle(
    title + 
      (GetReadOnly() ? " [" + _("Readonly") + "]": wxString(wxEmptyString)), 
    wxEmptyString);
}
