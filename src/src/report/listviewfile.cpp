////////////////////////////////////////////////////////////////////////////////
// Name:      listviewfile.cpp
// Purpose:   Implementation of class wex::report::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <pugixml.hpp>
#include <wex/config.h>
#include <wex/frame.h>
#include <wex/listitem.h>
#include <wex/log.h>
#include <wex/util.h>
#include <wex/report/listviewfile.h>
#include <wex/report/defs.h>
#include <wex/report/dir.h>
#include <wex/report/frame.h>

wex::report::file::file(
  const std::string& file, const listview_data& data)
  : report::listview(listview_data(data).type(listview_data::FILE))
  , wex::file(false) // do not open files in file_load and Save
  , m_add_itemsDialog(new item_dialog({
        {m_TextAddWhat,item::COMBOBOX, std::any(), 
           control_data().is_required(true)},
        {m_TextInFolder,item::COMBOBOX_DIR, std::any(), 
           control_data().is_required(true)},
        {std::set<std::string> {
          m_TextAddFiles, m_TextAddFolders, m_TextAddRecursive}}},
      window_data().
        title(_("Add Items").ToStdString()).
        button(wxAPPLY | wxCANCEL).id(wxID_ADD)))
{
  file_load(file);
  
  Bind(wxEVT_IDLE, &report::file::OnIdle, this);
  
  Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent& event) {
    event.Skip();

    // If no item has been selected, then show 
    // filename mod time in the statusbar.
    int flags = wxLIST_HITTEST_ONITEM;

    if (const int index = HitTest(wxPoint(event.GetX(), event.GetY()), flags); 
      index < 0)
    {
      log::status() << get_filename();
    }
  });
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    // Force at least one of the checkboxes to be checked.
    m_add_itemsDialog->force_checkbox_checked(_("Add"));
    if (GetSelectedItemCount() > 0)
    {
      wex::item item(m_add_itemsDialog->get_item(m_TextInFolder));
      wxComboBox* cb = (wxComboBox* )item.window();
      cb->SetValue(listitem(
        this, GetFirstSelected()).get_filename().get_path());
    }
    m_add_itemsDialog->Show();}, wxID_ADD);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    event.Skip();
    if (!get_filename().file_exists() || !get_filename().is_readonly())
    {
      m_ContentsChanged = true;
      frame::update_statusbar(this);
    }}, wxID_EDIT, wxID_REPLACE_ALL);
}

wex::report::file::~file()
{
  m_add_itemsDialog->Destroy();
}

void wex::report::file::add_items(
  const std::string& folder,
  const std::string& files,
  dir::type_t flags,
  bool detached)
{
  Unbind(wxEVT_IDLE, &file::OnIdle, this);
  
#ifdef __WXMSW__ 
  std::thread t([=] {
#endif
    const int old_count = GetItemCount();
    report::dir dir(this, folder, files, flags);
  
    dir.find_files();
    
    const int added = GetItemCount() - old_count;
    
    if (added > 0)
    {
      m_ContentsChanged = true;
  
      if (config("List/SortSync").get(true))
      {
        sort_column(sorted_column_no(), SORT_KEEP);
      }
    }
  
    log::status("Added") << added << "file(s)";
  
    Bind(wxEVT_IDLE, &file::OnIdle, this);
#ifdef __WXMSW__ 
    });
  if (detached)  
    t.detach();
  else
    t.join();
#endif
}

void wex::report::file::after_sorting()
{
  // Only if we are not sort syncing set contents changed.
  if (!config("List/SortSync").get(true))
  {
    m_ContentsChanged = true;
  }
}

void wex::report::file::build_popup_menu(wex::menu& menu)
{
  const bool ok =
     !get_filename().file_exists() || 
     (get_filename().file_exists() && !get_filename().is_readonly());

  menu.style().set(wex::menu::IS_READ_ONLY, ok);

  listview::build_popup_menu(menu);
    
  if (ok)
  {
    menu.append_separator();
    menu.append(wxID_ADD);
  }
}

bool wex::report::file::do_file_load(bool synced)
{
  pugi::xml_document doc;
  const pugi::xml_parse_result result = doc.load_file(
    get_filename().data().string().c_str(),
    pugi::parse_default | pugi::parse_comments);

  if (!result)
  {
    xml_error(get_filename(), &result);
    return false;
  }

  clear();

// TODO: threading this on MSW does not work.
#ifdef FIX__WXMSW__ 
  std::thread t([=] {
#endif

  for (const auto& child: doc.document_element().children())
  {
    if (const std::string value = child.text().get(); 
      strcmp(child.name(), "file") == 0)
    {
      listitem(this, value).insert();
    }
    else if (strcmp(child.name(), "folder") == 0)
    {
      listitem(this, value, child.attribute("extensions").value()).insert();
    }

    if (interruptable::is_cancelled()) break;
  }

  if (synced)
  {
    log::status() << get_filename();
  }

  get_frame()->set_recent_project(get_filename());
  
#ifdef FIX__WXMSW__ 
    });
  if (detached)  
    t.detach();
  else
    t.join();
#endif

  log::verbose("opened", 1) << get_filename();

  return true;
}

void wex::report::file::do_file_new()
{
  clear();
}

void wex::report::file::do_file_save(bool save_as)
{
  pugi::xml_document doc;

  doc.load_string(std::string("\
    <files>\n\
    <!-- " + wxTheApp->GetAppDisplayName().ToStdString() + " project " + 
      get_filename().fullname() + 
      " "  + wxDateTime::Now().Format().ToStdString().c_str() + "-->\n\
    </files>\n").c_str(),
       pugi::parse_default | pugi::parse_comments);
  
  pugi::xml_node root = doc.document_element();

  for (int i = 0; i < GetItemCount(); i++)
  {
    const wex::path fn = listitem(this, i).get_filename();
    
    pugi::xml_node node = root.append_child(fn.file_exists() ? "file": "folder");
    node.text().set(fn.data().string().c_str());

    if (fn.dir_exists())
    {
      node.append_attribute("extensions") = 
        get_item_text(i, _("Type").ToStdString()).c_str();
    }
  }
  
  if (doc.save_file(get_filename().data().string().c_str()))
  {
    log::verbose("saved", 1) << get_filename();
  }
  else
  {
    log("xml save") << get_filename();
  }
}

bool wex::report::file::item_from_text(const std::string& text)
{
  bool result = false;
  
  if (listview::item_from_text(text))
  {
    m_ContentsChanged = true;
    result = true;
    
    if (config("List/SortSync").get(true))
    {
      sort_column(sorted_column_no(), SORT_KEEP);
    }
    else
    {
      sort_column_reset();
    }
  }
  
  return result;
}

void wex::report::file::OnIdle(wxIdleEvent& event)
{
  event.Skip();
  
  if (
    IsShown() &&
    GetItemCount() > 0 &&
    config("AllowSync").get(true))
  {
    check_sync();
  }
}
