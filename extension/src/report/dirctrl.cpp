////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.cpp
// Purpose:   Implementation of class wxExGenericDirCtrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/textfile.h>
#include <wx/extension/filename.h>
#include <wx/extension/menu.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/tostring.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>

#if wxUSE_DIRDLG

#define GET_VECTOR_FILES \
  const std::vector< std::string > files(wxExToVectorString(*this).Get()); \
  if (files.empty()) \
  {                  \
    event.Skip();    \
    return;          \
  }

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
      wxDirDialogDefaultFolderStr,
      pos,
      size,
      style,
      (filter.empty() ? "*": filter),
      defaultFilter,
      name)
{
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      wxExVCSExecute(frame, 
        event.GetId() - ID_EDIT_VCS_LOWEST - 1, wxExToVectorString(*this).Get());},
    ID_EDIT_VCS_LOWEST + 1, 
    ID_EDIT_VCS_HIGHEST - 1);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxBusyCursor wait;
    std::string clipboard;
    for (const auto& it : wxExToVectorString(*this).Get())
    {
      clipboard += it + wxTextFile::GetEOL();
    }
    wxExClipboardAdd(clipboard);}, ID_TREE_COPY);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExOpenFiles(frame, 
      wxExToVectorString(*this).Get(), 0, wxDIR_FILES); // only files in this dir
    }, ID_EDIT_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    GET_VECTOR_FILES
    wxExMake(files[0]);}, ID_TREE_RUN_MAKE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frame->FindInFiles(wxExToVectorString(*this).Get(), event.GetId());}, 
    ID_TOOL_REPORT_FIND, ID_TOOL_REPORT_REPLACE);
    
  Bind(wxEVT_TREE_ITEM_ACTIVATED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    const wxFileName fn(files[0]);
    
    if (!fn.FileExists() && fn.DirExists())
    {
      if (!GetTreeCtrl()->IsExpanded(event.GetItem()))
      {
        ExpandAndSelectPath(files[0]);
      }
      else
      {
        CollapsePath(files[0]);
      }
    }
    else
    {
      wxExOpenFiles(frame, files, 0, wxDIR_FILES); // only files in this dir
    }});
  
  Bind(wxEVT_TREE_ITEM_MENU, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    const wxExFileName filename(files[0]);
  
    wxExMenu menu; // uses AppendVCS
    
    if (filename.FileExists())
    {
      menu.Append(ID_EDIT_OPEN, _("&Open"));
      menu.AppendSeparator();
    }
    
    menu.Append(ID_TREE_COPY,
      wxGetStockLabel(wxID_COPY), wxEmptyString, wxART_COPY);

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
      wxExEllipsed(frame->GetFindInCaption(ID_TOOL_REPORT_FIND)));

    menu.Append(ID_TOOL_REPORT_REPLACE, 
      wxExEllipsed(frame->GetFindInCaption(ID_TOOL_REPORT_REPLACE)));
      
    PopupMenu(&menu);});
  
  Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    wxExLogStatus(wxFileName(files[0]), STAT_FULLPATH);});
}

void wxExGenericDirCtrl::ExpandAndSelectPath(const wxString& path)
{
  ExpandPath(path);
  SelectPath(path);
}
#endif
