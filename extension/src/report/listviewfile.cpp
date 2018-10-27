////////////////////////////////////////////////////////////////////////////////
// Name:      listview_file.cpp
// Purpose:   Implementation of class wex::listview_file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <pugixml.hpp>
#include <wex/config.h>
#include <wex/frame.h>
#include <wex/itemdlg.h>
#include <wex/listitem.h>
#include <wex/log.h>
#include <wex/util.h>
#include <wex/report/listviewfile.h>
#include <wex/report/defs.h>
#include <wex/report/dir.h>
#include <wex/report/frame.h>
#include <easylogging++.h>

wex::listview_file::listview_file(
  const std::string& file, const listview_data& data)
  : history_listview(listview_data(data).Type(listview_data::FILE))
  , wex::file(false) // do not open files in FileLoad and Save
  , m_AddItemsDialog(new item_dialog({
        {m_TextAddWhat,item::COMBOBOX, std::any(), 
           control_data().Required(true)},
        {m_TextInFolder,item::COMBOBOX_DIR, std::any(), 
           control_data().Required(true)},
        {std::set<std::string> {
          m_TextAddFiles, m_TextAddFolders, m_TextAddRecursive}}},
      window_data().
        Title(_("Add Items").ToStdString()).
        Button(wxAPPLY | wxCANCEL).Id(wxID_ADD)))
{
  FileLoad(file);
  
  Bind(wxEVT_IDLE, &listview_file::OnIdle, this);
  
  Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent& event) {
    event.Skip();

    // If no item has been selected, then show 
    // filename mod time in the statusbar.
    int flags = wxLIST_HITTEST_ONITEM;

    if (const int index = HitTest(wxPoint(event.GetX(), event.GetY()), flags); 
      index < 0)
    {
      log_status(GetFileName());
    }
  });
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    // Force at least one of the checkboxes to be checked.
    m_AddItemsDialog->ForceCheckBoxChecked(_("Add"));
    if (GetSelectedItemCount() > 0)
    {
      wex::item item(m_AddItemsDialog->GetItem(m_TextInFolder));
      wxComboBox* cb = (wxComboBox* )item.GetWindow();
      cb->SetValue(listitem(
        this, GetFirstSelected()).GetFileName().GetPath());
    }
    m_AddItemsDialog->Show();}, wxID_ADD);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    event.Skip();
    if (!GetFileName().FileExists() || !GetFileName().IsReadOnly())
    {
      m_ContentsChanged = true;
      frame::UpdateStatusBar(this);
    }}, wxID_EDIT, wxID_REPLACE_ALL);
}

wex::listview_file::~listview_file()
{
  m_AddItemsDialog->Destroy();
}

void wex::listview_file::AddItems(
  const std::string& folder,
  const std::string& files,
  long flags,
  bool detached)
{
  Unbind(wxEVT_IDLE, &listview_file::OnIdle, this);
  
#ifdef __WXMSW__ 
  std::thread t([=] {
#endif
    const int old_count = GetItemCount();
    listview_dir dir(this, folder, files, flags);
  
    dir.FindFiles();
    
    const int added = GetItemCount() - old_count;
    
    if (added > 0)
    {
      m_ContentsChanged = true;
  
      if (config("List/SortSync").get(true))
      {
        SortColumn(GetSortedColumnNo(), SORT_KEEP);
      }
    }
  
    const std::string text = 
      _("Added") + " " + std::to_string(added) + " " + _("file(s)");
  
    wxLogStatus(text.c_str());
    
    Bind(wxEVT_IDLE, &listview_file::OnIdle, this);
#ifdef __WXMSW__ 
    });
  if (detached)  
    t.detach();
  else
    t.join();
#endif
}

void wex::listview_file::AfterSorting()
{
  // Only if we are not sort syncing set contents changed.
  if (!config("List/SortSync").get(true))
  {
    m_ContentsChanged = true;
  }
}

void wex::listview_file::BuildPopupMenu(wex::menu& menu)
{
  const bool ok =
     !GetFileName().FileExists() || 
     (GetFileName().FileExists() && !GetFileName().IsReadOnly());
  const auto style = menu.GetStyle() | (!ok ? wex::menu::IS_READ_ONLY: 0);

  menu.SetStyle(style);

  history_listview::BuildPopupMenu(menu);
    
  if (ok)
  {
    menu.AppendSeparator();
    menu.Append(wxID_ADD);
  }
}

bool wex::listview_file::DoFileLoad(bool synced)
{
  pugi::xml_document doc;
  const pugi::xml_parse_result result = doc.load_file(
    GetFileName().Path().string().c_str(),
    pugi::parse_default | pugi::parse_comments);

  if (!result)
  {
    xml_error(GetFileName(), &result);
    return false;
  }

  EditClearAll();

// TODO: threading this on MSW does not work.
#ifdef FIX__WXMSW__ 
  std::thread t([=] {
#endif

  for (const auto& child: doc.document_element().children())
  {
    if (const std::string value = child.text().get(); 
      strcmp(child.name(), "file") == 0)
    {
      listitem(this, value).Insert();
    }
    else if (strcmp(child.name(), "folder") == 0)
    {
      listitem(this, value, child.attribute("extensions").value()).Insert();
    }

    if (interruptable::Cancelled()) break;
  }

  if (synced)
  {
    log_status(GetFileName(), STAT_SYNC | STAT_FULLPATH);
  }

  GetFrame()->SetRecentProject(GetFileName());
  
#ifdef FIX__WXMSW__ 
    });
  if (detached)  
    t.detach();
  else
    t.join();
#endif

  VLOG(1) << "opened: " << GetFileName().Path().string();

  return true;
}

void wex::listview_file::DoFileNew()
{
  EditClearAll();
}

void wex::listview_file::DoFileSave(bool save_as)
{
  pugi::xml_document doc;

  doc.load_string(std::string("\
    <files>\n\
    <!-- " + wxTheApp->GetAppDisplayName().ToStdString() + " project " + 
      GetFileName().GetFullName() + 
      " "  + wxDateTime::Now().Format().ToStdString().c_str() + "-->\n\
    </files>\n").c_str(),
       pugi::parse_default | pugi::parse_comments);
  
  pugi::xml_node root = doc.document_element();

  for (int i = 0; i < GetItemCount(); i++)
  {
    const wex::path fn = listitem(this, i).GetFileName();
    
    pugi::xml_node node = root.append_child(fn.FileExists() ? "file": "folder");
    node.text().set(fn.Path().string().c_str());

    if (fn.DirExists())
    {
      node.append_attribute("extensions") = 
        GetItemText(i, _("Type").ToStdString()).c_str();
    }
  }
  
  if (doc.save_file(GetFileName().Path().string().c_str()))
  {
    VLOG(1) << "saved: " << GetFileName().Path().string();
  }
  else
  {
    log("xml save") << GetFileName().Path().string();
  }
}

bool wex::listview_file::ItemFromText(const std::string& text)
{
  bool result = false;
  
  if (listview::ItemFromText(text))
  {
    m_ContentsChanged = true;
    result = true;
    
    if (config("List/SortSync").get(true))
    {
      SortColumn(GetSortedColumnNo(), SORT_KEEP);
    }
    else
    {
      SortColumnReset();
    }
  }
  
  return result;
}

void wex::listview_file::OnIdle(wxIdleEvent& event)
{
  event.Skip();
  
  if (
    IsShown() &&
    GetItemCount() > 0 &&
    config("AllowSync").get(true))
  {
    CheckSync();
  }
}
