////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.cpp
// Purpose:   Implementation of class 'wxExGenericDirCtrl'
// Author:    Anton van Wezenbeek
// Created:   2010-08-16
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/log.h>
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/extension/menu.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/textfile.h>
#include <wx/extension/report/util.h>

#if wxUSE_DIRDLG

BEGIN_EVENT_TABLE(wxExGenericDirCtrl, wxGenericDirCtrl)
  EVT_MENU_RANGE(
    ID_EDIT_VCS_LOWEST, 
    ID_EDIT_VCS_HIGHEST, 
    wxExGenericDirCtrl::OnCommand)
  EVT_MENU_RANGE(ID_TREE_OPEN, ID_TREE_RUN_MAKE, wxExGenericDirCtrl::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, wxExGenericDirCtrl::OnCommand)
  EVT_TREE_ITEM_ACTIVATED(wxID_TREECTRL, wxExGenericDirCtrl::OnTree)
  EVT_TREE_ITEM_MENU(wxID_TREECTRL, wxExGenericDirCtrl::OnTree)
  EVT_TREE_SEL_CHANGED(wxID_TREECTRL, wxExGenericDirCtrl::OnTree)
END_EVENT_TABLE()

wxExGenericDirCtrl::wxExGenericDirCtrl(
  wxWindow *parent, 
  wxExFrameWithHistory* frame,
  const wxWindowID id, 
  const wxPoint &pos, 
  const wxSize &size, 
  long style, 
  const wxString &filter, 
  int defaultFilter, 
  const wxString &name)
  : wxGenericDirCtrl(
      parent,
      id,
      wxFileName(frame->GetRecentFile()).GetFullPath(),
      pos,
      size,
      style,
      filter,
      defaultFilter,
      name)
  , m_Frame(frame)
{
}

void wxExGenericDirCtrl::OnCommand(wxCommandEvent& event)
{
  wxArrayString files;
  GetFilePaths(files);
    
  if (event.GetId() > ID_EDIT_VCS_LOWEST && 
      event.GetId() < ID_EDIT_VCS_HIGHEST)
  {
    wxExVCSExecute(m_Frame, event.GetId(), files);
  }
  else if (event.GetId() > ID_TOOL_LOWEST && event.GetId() < ID_TOOL_HIGHEST)
  {
    const wxExTool tool(event.GetId());
    
    if (m_Frame->FindInSelectionDialog(tool.GetId()) == wxID_CANCEL ||
       !wxExTextFileWithListView::SetupTool(tool, m_Frame))
    {
      return;
    }
    
    wxExStatistics<long> stats;

    for (auto i = 0; i < files.GetCount(); i++)
    {
      wxExTextFileWithListView file(wxExFileName(files[i]), tool);
      file.RunTool();
      stats += file.GetStatistics().GetElements();
    }
    
    tool.Log(&stats);
  }
  else switch (event.GetId())
  {
  case ID_TREE_COPY: 
    {
    wxBusyCursor wait;
    wxString clipboard;
    for (
      auto it = files.begin();
      it != files.end();
      it++)
    {
      clipboard += *it + wxTextFile::GetEOL();
    }
    wxExClipboardAdd(clipboard);
    }
  break;
  
  case ID_TREE_OPEN: 
    wxExOpenFiles(m_Frame, files, 0, wxDIR_FILES); // only files in this dir
  break;
  
  case ID_TREE_RUN_MAKE: 
    wxExMake(m_Frame, files[0]);
    break;
    
  default: wxFAIL;
  }
}

void wxExGenericDirCtrl::OnTree(wxTreeEvent& event)
{
  wxArrayString files;
  GetFilePaths(files);

  if (files.empty()) 
  {
    event.Skip();
    return;
  }

  if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_MENU)
  {
    wxExMenu menu; // uses AppendVCS
    menu.Append(ID_TREE_OPEN, _("&Open"));
    menu.AppendSeparator();
    menu.Append(ID_TREE_COPY,
      wxGetStockLabel(wxID_COPY), wxEmptyString, wxART_COPY);

    const wxExFileName filename(files[0]);
  
    if (wxExVCS::DirExists(filename))
    {
      menu.AppendSeparator();
      menu.AppendVCS(filename);
    }

    if (filename.GetLexer().GetScintillaLexer() == "makefile")
    {
      menu.AppendSeparator();
      menu.Append(ID_TREE_RUN_MAKE, "&Make");
    }

    menu.AppendSeparator();
    menu.Append(ID_TOOL_REPORT_FIND, 
      wxExEllipsed(m_Frame->GetFindInCaption(ID_TOOL_REPORT_FIND)));

    menu.Append(ID_TOOL_REPORT_REPLACE, 
      wxExEllipsed(m_Frame->GetFindInCaption(ID_TOOL_REPORT_REPLACE)));
      
    PopupMenu(&menu);
  }
  else if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_ACTIVATED)
  {
    wxExOpenFiles(m_Frame, files, 0, wxDIR_FILES); // only files in this dir
  }
  else if (event.GetEventType() ==  wxEVT_COMMAND_TREE_SEL_CHANGED)
  {
    wxExLogStatus(wxFileName(files[0]), STAT_FULLPATH);
  }
  else
  {
    wxFAIL;
  }
}
#endif
