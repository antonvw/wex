////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex::history_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
#include <wex/util.h>
#include <wex/report/frame.h>
#include <wex/report/defs.h>
#include <wex/report/dir.h>
#include <wex/report/listviewfile.h>
#include <wex/report/stream.h>

wex::history_frame::history_frame(
  size_t maxFiles,
  size_t maxProjects,
  const window_data& data)
  : managed_frame(maxFiles, data)
  , m_ProjectHistory(maxProjects, ID_RECENT_PROJECT_LOWEST, "RecentProject")
  , m_Info({
      find_replace_data::get()->text_match_word(),
      find_replace_data::get()->text_match_case(),
      find_replace_data::get()->text_regex()})
{
  // Take care of default value.
  if (!config(m_TextRecursive).exists())
  {
    config(m_TextRecursive).set(true); 
  }

  std::set<std::string> t(m_Info);
  t.insert(m_TextRecursive);
  
  const std::vector<item> f {
    {find_replace_data::get()->text_find(), 
       item::COMBOBOX, std::any(), control_data().is_required(true)},
    {m_TextInFiles, item::COMBOBOX, std::any(), control_data().is_required(true)},
    {m_TextInFolder, item::COMBOBOX_DIR, std::any(), control_data().is_required(true)},
    {t}};
  
  m_FiFDialog = new item_dialog(
    f,
    window_data().
      button(wxAPPLY | wxCANCEL).
      id(ID_FIND_IN_FILES).
      title(_("Find In Files").ToStdString()).
      style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));
    
  m_RiFDialog = new item_dialog({
      f.at(0),
      {find_replace_data::get()->text_replace_with(), item::COMBOBOX},
      f.at(1),
      f.at(2),
      {_("Max replacements"), -1, INT_MAX},
      // Match whole word does not work with replace.
      {{find_replace_data::get()->text_match_case(),
        find_replace_data::get()->text_regex(),
        m_TextRecursive}}},
    window_data().
      button(wxAPPLY | wxCANCEL).
      id(ID_REPLACE_IN_FILES).
      title(_("Replace In Files").ToStdString()).
      style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));

  Bind(wxEVT_IDLE, &history_frame::OnIdle, this);
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_ProjectHistory.save();
    event.Skip();});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ProjectHistory.clear();}, ID_CLEAR_PROJECTS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* project = get_project(); project != nullptr)
    {
      project->file_save();
    }}, ID_PROJECT_SAVE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!event.GetString().empty())
    {
      grep(event.GetString().ToStdString());
    }
    else if (m_FiFDialog != nullptr)
    {
      if (get_stc() != nullptr && !get_stc()->get_find_string().empty())
      {
        m_FiFDialog->reload(); 
      }
      m_FiFDialog->Show(); 
    }}, ID_TOOL_REPORT_FIND);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!event.GetString().empty())
    {
      sed(event.GetString().ToStdString());
    }
    else if (m_RiFDialog != nullptr)
    {
      if (get_stc() != nullptr && !get_stc()->get_find_string().empty())
      {
        m_RiFDialog->reload(); 
      }
      m_RiFDialog->Show();
    }}, ID_TOOL_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    DoRecent(m_ProjectHistory, event.GetId() - m_ProjectHistory.get_base_id(), stc_data::WIN_IS_PROJECT);},
    m_ProjectHistory.get_base_id(), m_ProjectHistory.get_base_id() + m_ProjectHistory.get_max_files());
}

void wex::history_frame::find_in_files(wxWindowID dialogid)
{
  const bool replace = (dialogid == ID_REPLACE_IN_FILES);
  const wex::tool tool = (replace ?
    ID_TOOL_REPLACE: ID_TOOL_REPORT_FIND);

  if (!listview_stream::setup_tool(tool, this)) return;

#ifdef __WXMSW__
  std::thread t([=]{
#endif
    wxLogStatus(GetFindReplaceInfoText(replace));
      
    Unbind(wxEVT_IDLE, &history_frame::OnIdle, this);
      
    dir::type_t type;
    type.set(dir::FILES);
    if (config(m_TextRecursive).get(true)) type.set(dir::RECURSIVE);

    if (tool_dir dir(
      tool,
      config(m_TextInFolder).firstof(),
      config(m_TextInFiles).firstof(),
      type);

      dir.find_files() >= 0)
    {
      log_status(tool.info(&dir.get_statistics().get_elements()));
    }
    
    Bind(wxEVT_IDLE, &history_frame::OnIdle, this);

#ifdef __WXMSW__
    });
  t.detach();
#endif
}

bool wex::history_frame::find_in_files(
  const std::vector< path > & files,
  int id,
  bool show_dialog,
  listview* report)
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
  
  if (!listview_stream::setup_tool(tool, this, report))
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
        if (listview_stream file(it, tool); file.run_tool())
        {
          stats += file.get_statistics().get_elements();
        }
      }
      else if (it.dir_exists())
      {
        tool_dir dir(
          tool, 
          it, 
          config(m_TextInFiles).firstof());
          
        dir.find_files();
        stats += dir.get_statistics().get_elements();
      }
    }
    
    log_status(tool.info(&stats));
    
#ifdef __WXMSW__
    });
  t.detach();
#endif
  
  return true;
}

int wex::history_frame::find_in_files_dialog(
  int id,
  bool add_in_files)
{
  if (get_stc() != nullptr)
  {
    get_stc()->get_find_string();
  }

  if (item_dialog({
      {find_replace_data::get()->text_find(), item::COMBOBOX, std::any(), control_data().is_required(true)}, 
      (add_in_files ? item(m_TextInFiles, item::COMBOBOX, std::any(), control_data().is_required(true)) : item()),
      (id == ID_TOOL_REPLACE ? item(find_replace_data::get()->text_replace_with(), item::COMBOBOX): item()),
      item(m_Info)},
    window_data().title(find_in_files_title(id))).ShowModal() == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  wxLogStatus(GetFindReplaceInfoText(id == ID_TOOL_REPLACE));
        
  return wxID_OK;
}

const std::string wex::history_frame::find_in_files_title(int id) const
{
  return (id == ID_TOOL_REPLACE ?
    _("Replace In Selection").ToStdString():
    _("Find In Selection").ToStdString());
}

const wxString wex::history_frame::GetFindReplaceInfoText(bool replace) const
{
  wxString log;
  
  // Printing a % in wxLogStatus gives assert
  if (
    find_replace_data::get()->get_find_string().find("%") == std::string::npos &&
    find_replace_data::get()->get_replace_string().find("%") == std::string::npos )
  {
    log = _("Searching for") + ": " + find_replace_data::get()->get_find_string();

    if (replace)
    {
      log += " " + _("replacing with") + ": " + find_replace_data::get()->get_replace_string();
    }
  }

  return log;
}

bool wex::history_frame::grep(const std::string& arg, bool sed)
{
  static wxString arg1 = config(m_TextInFolder).firstof();
  static wxString arg2 = config(m_TextInFiles).firstof();
  static dir::type_t arg3 = dir::FILES;

  if (get_stc() != nullptr)
  {
    get_stc()->get_find_string();
  }

  if (!cmdline(
    {{{"r", "recursive", "recursive"}, [&](bool on) {arg3.set(dir::RECURSIVE, on);}}},
    {},
    {{"rest", "match " + std::string(sed ? "replace": "") + " [extension] [folder]"}, 
       [&](const std::vector<std::string> & v) {
       size_t i = 0;
       find_replace_data::get()->set_find_string(v[i++]);
       if (sed) 
       {
         if (v.size() <= i) return false;
         find_replace_data::get()->set_replace_string(v[i++]);
       }
       arg2 = (v.size() > i ? 
         config(m_TextInFiles).firstof_write(v[i++]): 
         config(m_TextInFiles).firstof());
       arg1 = (v.size() > i ? 
         config(m_TextInFolder).firstof_write(v[i++]): 
         config(m_TextInFolder).firstof());
       return true;}}).parse(std::string(sed ? ":sed": ":grep") + " " + arg))
  {
    return false;
  }
  
  if (arg1.empty() || arg2.empty())
  {
    log("empty arguments") << arg1.ToStdString() << arg2.ToStdString();
    return false;
  }
  
  const wex::tool tool = (sed ?
    ID_TOOL_REPLACE:
    ID_TOOL_REPORT_FIND);

  if (!listview_stream::setup_tool(tool, this))
  {
    return false;
  }

#ifdef __WXMSW__
  std::thread t([=]{
#endif
    if (auto* stc = get_stc(); stc != nullptr)
      path::current(stc->get_filename().get_path());
    find_replace_data::get()->set_use_regex(true);
    wxLogStatus(GetFindReplaceInfoText());
    Unbind(wxEVT_IDLE, &history_frame::OnIdle, this);

    tool_dir dir(tool, arg1.ToStdString(), arg2.ToStdString(), arg3);
    dir.find_files();

    log_status(tool.info(&dir.get_statistics().get_elements()));
    Bind(wxEVT_IDLE, &history_frame::OnIdle, this);
  
#ifdef __WXMSW__
    });
  t.detach();
#endif
  
  return true;
}

void wex::history_frame::on_command_item_dialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_CANCEL:
      if (interruptable::cancel())
      {
        wxLogStatus(_("Cancelled"));
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
              config(get_project()->text_infolder()).firstof(),
              config(get_project()->text_addwhat()).firstof(),
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

void wex::history_frame::OnIdle(wxIdleEvent& event)
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

void wex::history_frame::set_recent_file(const wex::path& path)
{
  managed_frame::set_recent_file(path);
  
  if (m_FileHistoryList != nullptr && path.file_exists())
  {
    listitem(m_FileHistoryList, path).insert(0);

    if (m_FileHistoryList->GetItemCount() > 1)
    {
      for (auto i = m_FileHistoryList->GetItemCount() - 1; i >= 1 ; i--)
      {
        if (listitem item(m_FileHistoryList, i); item.get_filename() == path)
        {
          item.erase();
        }
      }
    }
  }
}

void wex::history_frame::use_file_history_list(listview* list)
{
  assert(list->data().type() == listview_data::HISTORY);
  
  m_FileHistoryList = list;
  m_FileHistoryList->Hide();

  // Add all (existing) items from FileHistory.
  for (size_t i = 0; i < file_history().size(); i++)
  {
    if (listitem item(m_FileHistoryList, 
      file_history().get_history_file(i));
      item.get_filename().stat().is_ok())
    {
      item.insert();
    }
  }
}
