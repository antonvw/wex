////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.cpp
// Purpose:   Implementation of class wxExGenericDirCtrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/extension/menu.h>
#include <wx/extension/path.h>
#include <wx/extension/tostring.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/frame.h>

#if wxUSE_DIRDLG

#define GET_VECTOR_FILES \
  const auto files(wxExToVectorPath(*this).Get()); \
  if (files.empty()) \
  {                  \
    event.Skip();    \
    return;          \
  }

wxExGenericDirCtrl::wxExGenericDirCtrl(
  wxExFrameWithHistory* frame,
  const wxString &filter, 
  int defaultFilter, 
  const wxExWindowData& data)
  : wxGenericDirCtrl(
      data.Parent(),
      data.Id(),
      wxDirDialogDefaultFolderStr,
      data.Pos(),
      data.Size(),
      data.Style(),
      (filter.empty() ? "*": filter),
      defaultFilter,
      data.Name())
{
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      wxExVCSExecute(frame, 
        event.GetId() - ID_EDIT_VCS_LOWEST - 1, wxExToVectorPath(*this).Get());},
    ID_EDIT_VCS_LOWEST + 1, 
    ID_EDIT_VCS_HIGHEST - 1);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxBusyCursor wait;
    std::string clipboard;
    for (const auto& it : wxExToVectorString(*this).Get())
    {
      clipboard += it + "\n";
    }
    wxExClipboardAdd(clipboard);}, ID_TREE_COPY);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExOpenFiles(frame, 
      wxExToVectorPath(*this).Get(), wxExSTCData(), DIR_FILES);
    }, ID_EDIT_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    GET_VECTOR_FILES
    wxExMake(files[0]);}, ID_TREE_RUN_MAKE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frame->FindInFiles(wxExToVectorPath(*this).Get(), event.GetId());}, 
    ID_TOOL_REPORT_FIND);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frame->FindInFiles(wxExToVectorPath(*this).Get(), event.GetId());}, 
    ID_TOOL_REPLACE);
    
  Bind(wxEVT_TREE_ITEM_ACTIVATED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    
    if (const wxExPath fn(files[0]); !fn.FileExists() && fn.DirExists())
    {
      if (!GetTreeCtrl()->IsExpanded(event.GetItem()))
      {
        ExpandAndSelectPath(files[0]);
      }
      else
      {
        CollapsePath(files[0].Path().string());
      }
    }
    else
    {
      wxExOpenFiles(frame, files, wxExSTCData(), DIR_FILES);
    }});
  
  Bind(wxEVT_TREE_ITEM_MENU, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    const wxExPath filename(files[0]);
  
    wxExMenu menu; // uses AppendVCS
    
    if (filename.FileExists())
    {
      menu.Append(ID_EDIT_OPEN, _("&Open"));
      menu.AppendSeparator();
    }
    
    menu.Append(ID_TREE_COPY,
      wxGetStockLabel(wxID_COPY), std::string(), wxART_COPY);

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

    menu.Append(ID_TOOL_REPLACE, 
      wxExEllipsed(frame->GetFindInCaption(ID_TOOL_REPLACE)));
      
    PopupMenu(&menu);});
  
  Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    wxExLogStatus(files[0], STAT_FULLPATH);});
}

void wxExGenericDirCtrl::ExpandAndSelectPath(const wxExPath& path)
{
  ExpandPath(path.Path().string());
  SelectPath(path.Path().string());
}
#endif
