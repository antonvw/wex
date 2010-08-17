////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.cpp
// Purpose:   Implementation of class 'wxExGenericDirCtrl'
// Author:    Anton van Wezenbeek
// Created:   2010-08-16
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/extension/frame.h>
#include <wx/extension/menu.h>
#include <wx/extension/textfile.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/util.h>

#if wxUSE_DIRDLG

BEGIN_EVENT_TABLE(wxExGenericDirCtrl, wxGenericDirCtrl)
  EVT_MENU_RANGE(
    ID_EDIT_VCS_LOWEST, 
    ID_EDIT_VCS_HIGHEST, 
    wxExGenericDirCtrl::OnCommand)
  EVT_MENU_RANGE(ID_TREE_OPEN, ID_TREE_RUN_MAKE, wxExGenericDirCtrl::OnCommand)
  EVT_TREE_ITEM_ACTIVATED(wxID_TREECTRL, wxExGenericDirCtrl::OnTree)
  EVT_TREE_ITEM_RIGHT_CLICK(wxID_TREECTRL, wxExGenericDirCtrl::OnTree)
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
  if (event.GetId() > ID_EDIT_VCS_LOWEST && 
      event.GetId() < ID_EDIT_VCS_HIGHEST)
  {
    wxArrayString files;
    GetFilePaths(files);
    wxExVCSExecute(m_Frame, event.GetId(), wxExFileName(files[0]));
  }
  else switch (event.GetId())
  {
  case ID_TREE_COPY: 
    {
    wxBusyCursor wait;
    wxArrayString files;
    GetFilePaths(files);
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
    {
    wxArrayString files;
    GetFilePaths(files);
    wxExOpenFiles(m_Frame, files, 0, wxDIR_FILES); // only files in this dir
    }
  break;
  
  case ID_TREE_RUN_MAKE: 
    wxExMake(m_Frame, wxFileName(GetFilePath()));
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

  if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK)
  {
    wxExMenu menu;
    menu.Append(ID_TREE_OPEN, _("&Open"));
    menu.AppendSeparator();
    menu.Append(ID_TREE_COPY,
      wxGetStockLabel(wxID_COPY), wxEmptyString, wxART_COPY);

    const wxString selection = files[0];
    const wxExFileName filename(selection);
  
    if (wxExVCS::Get()->DirExists(filename))
    {
      menu.AppendSeparator();
      menu.AppendVCS();
    }

    if (filename.GetLexer().GetScintillaLexer() == "makefile")
    {
      menu.AppendSeparator();
      menu.Append(ID_TREE_RUN_MAKE, "&Make");
    }

    PopupMenu(&menu);
  }
  else if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_ACTIVATED)
  {
    wxExOpenFiles(m_Frame, files, 0, wxDIR_FILES); // only files in this dir
  }
  else
  {
    wxFAIL;
  }
}
#endif
