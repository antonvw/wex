////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.cpp
// Purpose:   Implementation of class wex::report::dirctrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
  const auto files(wex::to_vector_path(*this).get()); \
  if (files.empty()) \
  {                  \
    event.Skip();    \
    return;          \
  }

const int idShowHidden = wxWindow::NewControlId();

wex::report::dirctrl::dirctrl(
  frame* frame,
  const std::string &filter, 
  int defaultFilter, 
  const window_data& data)
  : wxGenericDirCtrl(
      data.parent(),
      data.id(),
      wxDirDialogDefaultFolderStr,
      data.pos(),
      data.size(),
      data.style(),
      (filter.empty() ? "*": filter),
      defaultFilter,
      data.name())
{
  if (config(_("Show hidden")).get(false))
  {
    ShowHidden(true);
  }
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      vcs_execute(frame, 
        event.GetId() - ID_EDIT_VCS_LOWEST - 1, to_vector_path(*this).get());},
    ID_EDIT_VCS_LOWEST + 1, 
    ID_EDIT_VCS_HIGHEST - 1);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxBusyCursor wait;
    std::string clipboard;
    const auto v (to_vector_string(*this).get());
    for (const auto& it : v)
    {
      clipboard += it + "\n";
    }
    clipboard_add(clipboard);}, ID_TREE_COPY);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    open_files(frame, 
      to_vector_path(*this).get(), 
        stc_data(), dir::type_t().set(dir::FILES));
    }, ID_EDIT_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    GET_VECTOR_FILES
    make(files[0]);}, ID_TREE_RUN_MAKE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frame->find_in_files(to_vector_path(*this).get(), event.GetId());}, 
    ID_TOOL_REPORT_FIND);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frame->find_in_files(to_vector_path(*this).get(), event.GetId());}, 
    ID_TOOL_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    ShowHidden(!config(_("Show hidden")).get(false));
    config(_("Show hidden")).set(!config(_("Show hidden")).get(false));},
    idShowHidden);

  Bind(wxEVT_TREE_ITEM_ACTIVATED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    
    if (const wex::path fn(files[0]); !fn.file_exists() && fn.dir_exists())
    {
      if (!GetTreeCtrl()->IsExpanded(event.GetItem()))
      {
        expand_and_select_path(files[0]);
      }
      else
      {
        CollapsePath(files[0].data().string());
      }
    }
    else
    {
      open_files(frame, files, stc_data(), dir::type_t().set(dir::FILES));
    }});
  
  Bind(wxEVT_TREE_ITEM_MENU, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    const wex::path filename(files[0]);
  
    wex::menu menu;
    
    if (filename.file_exists())
    {
      menu.append(ID_EDIT_OPEN, _("&Open"));
      menu.append_separator();
    }
    
    menu.append(ID_TREE_COPY,
      wxGetStockLabel(wxID_COPY), std::string(), wxART_COPY);

    if (vcs::dir_exists(filename))
    {
      menu.append_separator();
      menu.append_vcs(filename);
    }

    if (filename.lexer().scintilla_lexer() == "makefile")
    {
      menu.append_separator();
      menu.append(ID_TREE_RUN_MAKE, "&Make");
    }

    menu.append_separator();
    menu.append(ID_TOOL_REPORT_FIND, 
      ellipsed(frame->find_in_files_title(ID_TOOL_REPORT_FIND)));
    menu.append(ID_TOOL_REPLACE, 
      ellipsed(frame->find_in_files_title(ID_TOOL_REPLACE)));

    menu.append_separator();
    auto* item = menu.AppendCheckItem(idShowHidden, _("Show hidden"));
    if (config(_("Show hidden")).get(false))
      item->Check();
      
    PopupMenu(&menu);});
  
  Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent& event) {
    GET_VECTOR_FILES
    log::status() << files[0];});
}

void wex::report::dirctrl::expand_and_select_path(const wex::path& path)
{
  ExpandPath(path.data().string());
  SelectPath(path.data().string());
}
#endif
