////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.cpp
// Purpose:   Implementation of class wex::dirctrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stockitem.h> // for wxGetStockLabel
#include <wex/menu.h>
#include <wex/path.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wex/report/defs.h>
#include <wex/report/dirctrl.h>
#include <wex/report/frame.h>

#if wxUSE_DIRDLG

#define GET_VECTOR_FILES \
  const auto files(wex::to_vector_path(*this).Get()); \
  if (files.empty()) \
  {                  \
    event.Skip();    \
    return;          \
  }

wex::dirctrl::dirctrl(
  history_frame* frame,
  const wxString &filter, 
  int defaultFilter, 
  const window_data& data)
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
      vcs_execute(frame, 
        event.GetId() - ID_EDIT_VCS_LOWEST - 1, to_vector_path(*this).Get());},
    ID_EDIT_VCS_LOWEST + 1, 
    ID_EDIT_VCS_HIGHEST - 1);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxBusyCursor wait;
    std::string clipboard;
    for (const auto& it : to_vector_string(*this).Get())
    {
      clipboard += it + "\n";
    }
    clipboard_add(clipboard);}, ID_TREE_COPY);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    open_files(frame, 
      to_vector_path(*this).Get(), stc_data(), dir::FILES);
    }, ID_EDIT_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    GET_VECTOR_FILES
    make(files[0]);}, ID_TREE_RUN_MAKE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frame->FindInFiles(to_vector_path(*this).Get(), event.GetId());}, 
    ID_TOOL_REPORT_FIND);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frame->FindInFiles(to_vector_path(*this).Get(), event.GetId());}, 
    ID_TOOL_REPLACE);
    
  Bind(wxEVT_TREE_ITEM_ACTIVATED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    
    if (const wex::path fn(files[0]); !fn.FileExists() && fn.DirExists())
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
      open_files(frame, files, stc_data(), dir::FILES);
    }});
  
  Bind(wxEVT_TREE_ITEM_MENU, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    const wex::path filename(files[0]);
  
    wex::menu menu; // uses append_vcs
    
    if (filename.FileExists())
    {
      menu.Append(ID_EDIT_OPEN, _("&Open"));
      menu.AppendSeparator();
    }
    
    menu.Append(ID_TREE_COPY,
      wxGetStockLabel(wxID_COPY), std::string(), wxART_COPY);

    if (vcs::DirExists(filename))
    {
      menu.AppendSeparator();
      menu.append_vcs(filename);
    }

    if (filename.GetLexer().GetScintillaLexer() == "makefile")
    {
      menu.AppendSeparator();
      menu.Append(ID_TREE_RUN_MAKE, "&Make");
    }

    menu.AppendSeparator();
    menu.Append(ID_TOOL_REPORT_FIND, 
      ellipsed(frame->GetFindInCaption(ID_TOOL_REPORT_FIND)));

    menu.Append(ID_TOOL_REPLACE, 
      ellipsed(frame->GetFindInCaption(ID_TOOL_REPLACE)));
      
    PopupMenu(&menu);});
  
  Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    log_status(files[0], STAT_FULLPATH);});
}

void wex::dirctrl::ExpandAndSelectPath(const wex::path& path)
{
  ExpandPath(path.Path().string());
  SelectPath(path.Path().string());
}
#endif
