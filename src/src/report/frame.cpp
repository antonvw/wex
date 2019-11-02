////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex::report::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/itemdlg.h>
#include <wex/listitem.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/stcdlg.h>
#include <wex/util.h>
#include <wex/report/frame.h>
#include <wex/report/defs.h>
#include <wex/report/dir.h>
#include <wex/report/listviewfile.h>
#include <wex/report/stream.h>

wex::report::frame::frame(
  size_t maxFiles,
  size_t maxProjects,
  const window_data& data)
  : managed_frame(maxFiles, data)
  , m_project_history(maxProjects, ID_RECENT_PROJECT_LOWEST, "RecentProjects")
  , m_info({
      find_replace_data::get()->text_match_word(),
      find_replace_data::get()->text_match_case(),
      find_replace_data::get()->text_regex()})
{
  // Take care of default value.
  if (!config(m_text_recursive).exists())
  {
    config(m_text_recursive).set(true); 
  }

  std::set<std::string> t(m_info);
  t.insert(m_text_recursive);
  
  const std::vector<item> f {
    {find_replace_data::get()->text_find(), 
       item::COMBOBOX, std::any(), control_data().is_required(true)},
    {m_text_in_files, item::COMBOBOX, std::any(), control_data().is_required(true)},
    {m_text_in_folder, item::COMBOBOX_DIR, std::any(), control_data().is_required(true)},
    {t}};
  
  m_fif_dialog = new item_dialog(
    f,
    window_data().
      button(wxAPPLY | wxCANCEL).
      id(ID_FIND_IN_FILES).
      title(_("Find In Files")).
      style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));
    
  m_rif_dialog = new item_dialog({
      f.at(0),
      {find_replace_data::get()->text_replace_with(), item::COMBOBOX},
      f.at(1),
      f.at(2),
      {_("Max replacements"), -1, INT_MAX},
      // Match whole word does not work with replace.
      {{find_replace_data::get()->text_match_case(),
        find_replace_data::get()->text_regex(),
        m_text_recursive}}},
    window_data().
      button(wxAPPLY | wxCANCEL).
      id(ID_REPLACE_IN_FILES).
      title(_("Replace In Files")).
      style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));

  Bind(wxEVT_IDLE, &frame::on_idle, this);
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_project_history.save();
    event.Skip();});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_project_history.clear();}, ID_CLEAR_PROJECTS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* project = get_project(); project != nullptr)
    {
      project->file_save();
    }}, ID_PROJECT_SAVE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!event.GetString().empty())
    {
      grep(event.GetString());
    }
    else if (m_fif_dialog != nullptr)
    {
      if (get_stc() != nullptr && !get_stc()->get_find_string().empty())
      {
        m_fif_dialog->reload(); 
      }
      m_fif_dialog->Show(); 
    }}, ID_TOOL_REPORT_FIND);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!event.GetString().empty())
    {
      sed(event.GetString());
    }
    else if (m_rif_dialog != nullptr)
    {
      if (get_stc() != nullptr && !get_stc()->get_find_string().empty())
      {
        m_rif_dialog->reload(); 
      }
      m_rif_dialog->Show();
    }}, ID_TOOL_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    on_menu_history(m_project_history, 
      event.GetId() - m_project_history.get_base_id(), 
      wex::stc_data::window_t().set(stc_data::WIN_IS_PROJECT));},
    m_project_history.get_base_id(), m_project_history.get_base_id() + m_project_history.get_max_files());
}

const std::string find_replace_string(bool replace)
{
  std::string log;
  
  log = _("Searching for") + ": " + wex::find_replace_data::get()->get_find_string();

  if (replace)
  {
    log += " " + _("replacing with") + ": " + wex::find_replace_data::get()->get_replace_string();
  }

  return log;
}

void wex::report::frame::find_in_files(wxWindowID dialogid)
{
  const bool replace = (dialogid == ID_REPLACE_IN_FILES);
  const wex::tool tool = (replace ?
    ID_TOOL_REPLACE: ID_TOOL_REPORT_FIND);

  if (!stream::setup_tool(tool, this)) return;

#ifdef __WXMSW__
  std::thread t([=]{
#endif
    log::status(find_replace_string(replace));
      
    Unbind(wxEVT_IDLE, &frame::on_idle, this);
      
    dir::type_t type;
    type.set(dir::FILES);
    if (config(m_text_recursive).get(true)) type.set(dir::RECURSIVE);

    if (tool_dir dir(
      tool,
      config(m_text_in_folder).get_firstof(),
      config(m_text_in_files).get_firstof(),
      type);

      dir.find_files() >= 0)
    {
      log::status(tool.info(&dir.get_statistics().get_elements()));
    }
    
    Bind(wxEVT_IDLE, &frame::on_idle, this);

#ifdef __WXMSW__
    });
  t.detach();
#endif
}

bool wex::report::frame::find_in_files(
  const std::vector< path > & files,
  int id,
  bool show_dialog,
  wex::listview* report)
{
  if (files.empty())
  {
    return false;
  }
  
  const wex::tool tool(id);
  
  if (const wex::path filename(files[0]); show_dialog && find_in_files_dialog(
    tool.id(),
    filename.dir_exists() && !filename.file_exists()) == wxID_CANCEL)
  {
    return false;
  }
  
  if (!stream::setup_tool(tool, this, report))
  {
    return false;
  }
  
#ifdef __WXMSW__
  std::thread t([=]{
#endif
    statistics<int> stats;
    
    for (const auto& it : files)
    {
      if (it.file_exists())
      {
        if (stream file(it, tool); file.run_tool())
        {
          stats += file.get_statistics().get_elements();
        }
      }
      else if (it.dir_exists())
      {
        tool_dir dir(
          tool, 
          it, 
          config(m_text_in_files).get_firstof());
          
        dir.find_files();
        stats += dir.get_statistics().get_elements();
      }
    }
    
    log::status(tool.info(&stats));
    
#ifdef __WXMSW__
    });
  t.detach();
#endif
  
  return true;
}

int wex::report::frame::find_in_files_dialog(
  int id,
  bool add_in_files)
{
  if (get_stc() != nullptr)
  {
    get_stc()->get_find_string();
  }

  if (item_dialog({
      {find_replace_data::get()->text_find(), item::COMBOBOX, std::any(), control_data().is_required(true)}, 
      (add_in_files ? item(m_text_in_files, item::COMBOBOX, std::any(), control_data().is_required(true)) : item()),
      (id == ID_TOOL_REPLACE ? item(find_replace_data::get()->text_replace_with(), item::COMBOBOX): item()),
      item(m_info)},
    window_data().title(find_in_files_title(id))).ShowModal() == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  log::status(find_replace_string(id == ID_TOOL_REPLACE));
        
  return wxID_OK;
}

const std::string wex::report::frame::find_in_files_title(int id) const
{
  return (id == ID_TOOL_REPLACE ?
    _("Replace In Selection"):
    _("Find In Selection"));
}

bool wex::report::frame::grep(const std::string& arg, bool sed)
{
  static std::string arg1 = config(m_text_in_folder).get_firstof();
  static std::string arg2 = config(m_text_in_files).get_firstof();
  static dir::type_t arg3 = dir::FILES;

  if (get_stc() != nullptr)
  {
    get_stc()->get_find_string();
  }

  if (std::string help; !cmdline(
    {{{"recursive,r", "recursive"}, [&](bool on) {arg3.set(dir::RECURSIVE, on);}}},
    {},
    {{"rest", "match " + std::string(sed ? "replace": "") + " [extension] [folder]"}, 
       [&](const std::vector<std::string> & v) {
       size_t i = 0;
       find_replace_data::get()->set_find_string(v[i++]);
       if (sed) 
       {
         if (v.size() <= i) return;
         find_replace_data::get()->set_replace_string(v[i++]);
       }
       arg2 = (v.size() > i ? 
         config(m_text_in_files).set_firstof(v[i++]): 
         config(m_text_in_files).get_firstof());
       arg1 = (v.size() > i ? 
         config(m_text_in_folder).set_firstof(v[i++]): 
         config(m_text_in_folder).get_firstof());
       }}, false).parse(arg, help))
  {
    stc_entry_dialog(help).ShowModal();
    return false;
  }
  
  if (arg1.empty() || arg2.empty())
  {
    log("empty arguments") << arg1 << arg2;
    return false;
  }
  
  const wex::tool tool = (sed ?
    ID_TOOL_REPLACE:
    ID_TOOL_REPORT_FIND);

  if (!stream::setup_tool(tool, this))
  {
    return false;
  }

#ifdef __WXMSW__
  std::thread t([=]{
#endif
    if (auto* stc = get_stc(); stc != nullptr)
      path::current(stc->get_filename().get_path());
    find_replace_data::get()->set_use_regex(true);
    log::status(find_replace_string(false));
    Unbind(wxEVT_IDLE, &frame::on_idle, this);

    tool_dir dir(tool, arg1, arg2, arg3);
    dir.find_files();

    log::status(tool.info(&dir.get_statistics().get_elements()));
    Bind(wxEVT_IDLE, &frame::on_idle, this);
  
#ifdef __WXMSW__
    });
  t.detach();
#endif
  
  return true;
}

void wex::report::frame::on_command_item_dialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_CANCEL:
      if (interruptable::cancel())
      {
        log::status(_("Cancelled"));
      }
      break;

    case wxID_OK:
    case wxID_APPLY:
      switch (dialogid)
      {
        case wxID_ADD:
          if (get_project() != nullptr)
          {
            dir::type_t flags = 0;
          
            if (config(get_project()->text_addfiles()).get(true)) 
              flags.set(dir::FILES);
            if (config(get_project()->text_addrecursive()).get(true)) 
              flags.set(dir::RECURSIVE);
            if (config(get_project()->text_addfolders()).get(true)) 
              flags.set(dir::DIRS);

            get_project()->add_items(
              config(get_project()->text_infolder()).get_firstof(),
              config(get_project()->text_addwhat()).get_firstof(),
              flags);
          }
          break;

        case ID_FIND_IN_FILES:
        case ID_REPLACE_IN_FILES:
          find_in_files(dialogid);
          break;

        default: assert(0);
      }
      break;

    default: assert(0);
  }
}

void wex::report::frame::on_idle(wxIdleEvent& event)
{
  event.Skip();

  std::string title(GetTitle());
  const std::string indicator(" *");
  
  if (title.size() < indicator.size())
  {
    return;
  }
  
  auto* stc = get_stc();
  auto* project = get_project();

  if (const size_t pos = title.size() - indicator.size();
    (project != nullptr && project->get_contents_changed()) ||
     // using get_contents_changed gives assert in vcs dialog
    (stc != nullptr && stc->GetModify() && 
      !stc->data().flags().test(stc_data::WIN_NO_INDICATOR)))
  {
    // Project or editor changed, add indicator if not yet done.
    if (title.substr(pos) != indicator)
    {
      SetTitle(title + indicator);
    }
  }
  else
  {
    // Project or editor not changed, remove indicator if not yet done.
    if (title.substr(pos) == indicator)
    {
      SetTitle(title.erase(pos));
    }
  }
}

void wex::report::frame::set_recent_file(const wex::path& path)
{
  managed_frame::set_recent_file(path);
  
  if (m_file_history_listview != nullptr && path.file_exists())
  {
    listitem(m_file_history_listview, path).insert(0);

    if (m_file_history_listview->GetItemCount() > 1)
    {
      for (auto i = m_file_history_listview->GetItemCount() - 1; i >= 1 ; i--)
      {
        if (listitem item(m_file_history_listview, i); item.get_filename() == path)
        {
          item.erase();
        }
      }
    }
  }
}

void wex::report::frame::use_file_history_list(wex::listview* list)
{
  assert(list->data().type() == listview_data::HISTORY);
  
  m_file_history_listview = list;
  m_file_history_listview->Hide();

  // Add all (existing) items from FileHistory.
  for (size_t i = 0; i < file_history().size(); i++)
  {
    if (listitem item(m_file_history_listview, 
      file_history().get_history_file(i));
      item.get_filename().stat().is_ok())
    {
      item.insert();
    }
  }
}
