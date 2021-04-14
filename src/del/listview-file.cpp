////////////////////////////////////////////////////////////////////////////////
// Name:      listview-file.cpp
// Purpose:   Implementation of class wex::del::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <pugixml.hpp>
#include <thread>
#include <wex/config.h>
#include <wex/del/defs.h>
#include <wex/del/dir.h>
#include <wex/del/frame.h>
#include <wex/del/listview-file.h>
#include <wex/listitem.h>
#include <wex/log.h>
#include <wex/menu.h>
#include <wex/util.h>

wex::del::file::file(const std::string& file, const data::listview& data)
  : del::listview(data::listview(data).type(data::listview::FILE))
  , m_add_items_dialog(new item_dialog(
      {{m_text_add_what,
        item::COMBOBOX,
        get_frame()->default_extensions(),
        data::control().is_required(true)},
       {m_text_in_folder,
        item::COMBOBOX_DIR,
        std::list<std::string>{wxGetHomeDir().ToStdString()},
        data::control().is_required(true)},
       {std::set<std::string>{
         m_text_add_files,
         m_text_add_folders,
         m_text_add_recursive}}},
      data::window()
        .title(_("Add Items"))
        .button(wxAPPLY | wxCANCEL)
        .id(wxID_ADD)))
{
  file_load(file);

  Bind(wxEVT_IDLE, &del::file::on_idle, this);

  Bind(wxEVT_LEFT_DOWN, [=, this](wxMouseEvent& event) {
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

  Bind(
    wxEVT_MENU,
    [=, this](wxCommandEvent& event) {
      // Force at least one of the checkboxes to be checked.
      m_add_items_dialog->force_checkbox_checked(_("Add"));
      if (GetSelectedItemCount() > 0)
      {
        wex::item item(m_add_items_dialog->find(m_text_in_folder));
        auto*     cb = (wxComboBox*)item.window();
        cb->SetValue(
          listitem(this, GetFirstSelected()).get_filename().get_path());
      }
      m_add_items_dialog->Show();
    },
    wxID_ADD);

  Bind(
    wxEVT_MENU,
    [=, this](wxCommandEvent& event) {
      event.Skip();
      if (!get_filename().file_exists() || !get_filename().is_readonly())
      {
        m_contents_changed = true;
        get_frame()->update_statusbar(this);
      }
    },
    wxID_EDIT,
    wxID_REPLACE_ALL);
}

wex::del::file::~file()
{
  m_add_items_dialog->Destroy();
}

void wex::del::file::add_items(
  const std::string& folder,
  const std::string& files,
  data::dir::type_t  flags)
{
  Unbind(wxEVT_IDLE, &file::on_idle, this);

#ifdef __WXMSW__
  std::thread t([=, this] {
#endif
    const int old_count = GetItemCount();
    del::dir  dir(this, folder, data::dir().file_spec(files).type(flags));

    dir.find_files();

    const int added = GetItemCount() - old_count;

    if (added > 0)
    {
      m_contents_changed = true;

      if (config("list.SortSync").get(true))
      {
        sort_column(sorted_column_no(), SORT_KEEP);
      }
    }

    log::status("Added") << added << "file(s)";

    Bind(wxEVT_IDLE, &file::on_idle, this);
#ifdef __WXMSW__
  });
  t.detach();
#endif
}

void wex::del::file::after_sorting()
{
  // Only if we are not sort syncing set contents changed.
  if (!config("list.SortSync").get(true))
  {
    m_contents_changed = true;
  }
}

void wex::del::file::build_popup_menu(wex::menu& menu)
{
  const bool ro(get_filename().file_exists() && get_filename().is_readonly());

  if (ro)
  {
    menu.style().set(wex::menu::IS_READ_ONLY, ro);
  }

  listview::build_popup_menu(menu);

  if (!ro)
  {
    menu.append({{}, {wxID_ADD}});
  }
}

bool wex::del::file::do_file_load(bool synced)
{
  pugi::xml_document doc;

  if (const auto result = doc.load_file(
        get_filename().string().c_str(),
        pugi::parse_default | pugi::parse_comments); !result)
  {
    if (get_filename().stat().st_size == 0)
    {
      clear();
      return true;
    }
    else
    {
      xml_error(get_filename(), &result);
      return false;
    }
  }

  clear();

#ifdef FIX__WXMSW__
  std::thread t([=, this] {
#endif
    for (const auto& child : doc.document_element().children())
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

      if (interruptible::is_cancelled())
        break;
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

  log::info("opened") << get_filename();

  return true;
}

void wex::del::file::do_file_new()
{
  clear();
}

void wex::del::file::do_file_save(bool save_as)
{
  pugi::xml_document doc;

  doc.load_string(
    std::string(
      "\
    <files>\n\
    <!-- " +
      wxTheApp->GetAppDisplayName().ToStdString() + " project " +
      get_filename().fullname() + " " + now() + "-->\n\
    </files>\n")
      .c_str(),
    pugi::parse_default | pugi::parse_comments);

  auto root = doc.document_element();

  for (int i = 0; i < GetItemCount(); i++)
  {
    const wex::path fn = listitem(this, i).get_filename();

    auto node = root.append_child(fn.file_exists() ? "file" : "folder");
    node.text().set(fn.string().c_str());

    if (fn.dir_exists())
    {
      node.append_attribute("extensions") = get_item_text(i, _("Type")).c_str();
    }
  }

  if (doc.save_file(get_filename().string().c_str()))
  {
    log::info("saved") << get_filename();
  }
  else
  {
    log("xml save") << get_filename();
  }
}

bool wex::del::file::item_from_text(const std::string& text)
{
  bool result = false;

  if (listview::item_from_text(text))
  {
    m_contents_changed = true;
    result             = true;

    if (config("list.SortSync").get(true))
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

void wex::del::file::on_idle(wxIdleEvent& event)
{
  event.Skip();

  if (IsShown() && GetItemCount() > 0 && config("AllowSync").get(true))
  {
    check_sync();
  }
}
