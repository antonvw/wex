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
      find_replace_data::Get()->GetTextMatchWholeWord(),
      find_replace_data::Get()->GetTextMatchCase(),
      find_replace_data::Get()->GetTextRegEx()})
{
  // Take care of default value.
  if (!config(m_TextRecursive).exists())
  {
    config(m_TextRecursive).set(true); 
  }

  std::set<std::string> t(m_Info);
  t.insert(m_TextRecursive);
  
  const std::vector<item> f {
    {find_replace_data::Get()->GetTextFindWhat(), 
       item::COMBOBOX, std::any(), control_data().Required(true)},
    {m_TextInFiles, item::COMBOBOX, std::any(), control_data().Required(true)},
    {m_TextInFolder, item::COMBOBOX_DIR, std::any(), control_data().Required(true)},
    {t}};
  
  m_FiFDialog = new item_dialog(
    f,
    window_data().
      Button(wxAPPLY | wxCANCEL).
      Id(ID_FIND_IN_FILES).
      Title(_("Find In Files").ToStdString()).
      Style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));
    
  m_RiFDialog = new item_dialog({
      f.at(0),
      {find_replace_data::Get()->GetTextReplaceWith(), item::COMBOBOX},
      f.at(1),
      f.at(2),
      {_("Max replacements"), -1, INT_MAX},
      // Match whole word does not work with replace.
      {{find_replace_data::Get()->GetTextMatchCase(),
        find_replace_data::Get()->GetTextRegEx(),
        m_TextRecursive}}},
    window_data().
      Button(wxAPPLY | wxCANCEL).
      Id(ID_REPLACE_IN_FILES).
      Title(_("Replace In Files").ToStdString()).
      Style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));

  Bind(wxEVT_IDLE, &history_frame::OnIdle, this);
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_ProjectHistory.Save();
    event.Skip();});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ProjectHistory.Clear();}, ID_CLEAR_PROJECTS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* project = GetProject(); project != nullptr)
    {
      project->FileSave();
    }}, ID_PROJECT_SAVE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!event.GetString().empty())
    {
      Grep(event.GetString().ToStdString());
    }
    else if (m_FiFDialog != nullptr)
    {
      if (GetSTC() != nullptr && !GetSTC()->GetFindString().empty())
      {
        m_FiFDialog->Reload(); 
      }
      m_FiFDialog->Show(); 
    }}, ID_TOOL_REPORT_FIND);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!event.GetString().empty())
    {
      Sed(event.GetString().ToStdString());
    }
    else if (m_RiFDialog != nullptr)
    {
      if (GetSTC() != nullptr && !GetSTC()->GetFindString().empty())
      {
        m_RiFDialog->Reload(); 
      }
      m_RiFDialog->Show();
    }}, ID_TOOL_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    DoRecent(m_ProjectHistory, event.GetId() - m_ProjectHistory.GetBaseId(), stc_data::WIN_IS_PROJECT);},
    m_ProjectHistory.GetBaseId(), m_ProjectHistory.GetBaseId() + m_ProjectHistory.GetMaxFiles());
}

void wex::history_frame::FindInFiles(wxWindowID dialogid)
{
  const bool replace = (dialogid == ID_REPLACE_IN_FILES);
  const wex::tool tool = (replace ?
    ID_TOOL_REPLACE: ID_TOOL_REPORT_FIND);

  if (!listview_stream::SetupTool(tool, this)) return;

#ifdef __WXMSW__
  std::thread t([=]{
#endif
    wxLogStatus(GetFindReplaceInfoText(replace));
      
    Unbind(wxEVT_IDLE, &history_frame::OnIdle, this);

    if (tool_dir dir(
      tool,
      config(m_TextInFolder).firstof(),
      config(m_TextInFiles).firstof(),
      dir::FILES | (config(m_TextRecursive).get(true) ? dir::RECURSIVE : 0));
      dir.FindFiles() >= 0)
    {
      log_status(tool.Info(&dir.GetStatistics().GetElements()));
    }
    
    Bind(wxEVT_IDLE, &history_frame::OnIdle, this);

#ifdef __WXMSW__
    });
  t.detach();
#endif
}

bool wex::history_frame::FindInFiles(
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
  
  if (const wex::path filename(files[0]); show_dialog && FindInFilesDialog(
    tool.GetId(),
    filename.DirExists() && !filename.FileExists()) == wxID_CANCEL)
  {
    return false;
  }
  
  if (!listview_stream::SetupTool(tool, this, report))
  {
    return false;
  }
  
#ifdef __WXMSW__
  std::thread t([=]{
#endif
    statistics<int> stats;
    
    for (const auto& it : files)
    {
      if (it.FileExists())
      {
        if (listview_stream file(it, tool); file.RunTool())
        {
          stats += file.GetStatistics().GetElements();
        }
      }
      else if (it.DirExists())
      {
        tool_dir dir(
          tool, 
          it, 
          config(m_TextInFiles).firstof());
          
        dir.FindFiles();
        stats += dir.GetStatistics().GetElements();
      }
    }
    
    log_status(tool.Info(&stats));
    
#ifdef __WXMSW__
    });
  t.detach();
#endif
  
  return true;
}

int wex::history_frame::FindInFilesDialog(
  int id,
  bool add_in_files)
{
  if (GetSTC() != nullptr)
  {
    GetSTC()->GetFindString();
  }

  if (item_dialog({
      {find_replace_data::Get()->GetTextFindWhat(), item::COMBOBOX, std::any(), control_data().Required(true)}, 
      (add_in_files ? item(m_TextInFiles, item::COMBOBOX, std::any(), control_data().Required(true)) : item()),
      (id == ID_TOOL_REPLACE ? item(find_replace_data::Get()->GetTextReplaceWith(), item::COMBOBOX): item()),
      item(m_Info)},
    window_data().Title(GetFindInCaption(id))).ShowModal() == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  wxLogStatus(GetFindReplaceInfoText(id == ID_TOOL_REPLACE));
        
  return wxID_OK;
}

const std::string wex::history_frame::GetFindInCaption(int id) const
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
    find_replace_data::Get()->GetFindString().find("%") == std::string::npos &&
    find_replace_data::Get()->GetReplaceString().find("%") == std::string::npos )
  {
    log = _("Searching for") + ": " + find_replace_data::Get()->GetFindString();

    if (replace)
    {
      log += " " + _("replacing with") + ": " + find_replace_data::Get()->GetReplaceString();
    }
  }

  return log;
}

bool wex::history_frame::Grep(const std::string& arg, bool sed)
{
  static wxString arg1 = config(m_TextInFolder).firstof();
  static wxString arg2 = config(m_TextInFiles).firstof();
  static int arg3 = dir::FILES;

  if (GetSTC() != nullptr)
  {
    GetSTC()->GetFindString();
  }

  if (!cmdline(
    {{{"r", "recursive", "recursive"}, [&](bool on) {arg3 |= (on ? dir::RECURSIVE: 0);}}},
    {},
    {{"rest", "match " + std::string(sed ? "replace": "") + " [extension] [folder]"}, 
       [&](const std::vector<std::string> & v) {
       size_t i = 0;
       find_replace_data::Get()->SetFindString(v[i++]);
       if (sed) 
       {
         if (v.size() <= i) return false;
         find_replace_data::Get()->SetReplaceString(v[i++]);
       }
       arg2 = (v.size() > i ? 
         config(m_TextInFiles).firstof_write(v[i++]): 
         config(m_TextInFiles).firstof());
       arg1 = (v.size() > i ? 
         config(m_TextInFolder).firstof_write(v[i++]): 
         config(m_TextInFolder).firstof());
       return true;}}).Parse(std::string(sed ? ":sed": ":grep") + " " + arg))
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

  if (!listview_stream::SetupTool(tool, this))
  {
    return false;
  }

#ifdef __WXMSW__
  std::thread t([=]{
#endif
    if (auto* stc = GetSTC(); stc != nullptr)
      path::Current(stc->GetFileName().GetPath());
    find_replace_data::Get()->SetUseRegEx(true);
    wxLogStatus(GetFindReplaceInfoText());
    Unbind(wxEVT_IDLE, &history_frame::OnIdle, this);

    tool_dir dir(tool, arg1.ToStdString(), arg2.ToStdString(), arg3);
    dir.FindFiles();

    log_status(tool.Info(&dir.GetStatistics().GetElements()));
    Bind(wxEVT_IDLE, &history_frame::OnIdle, this);
  
#ifdef __WXMSW__
    });
  t.detach();
#endif
  
  return true;
}

void wex::history_frame::OnCommandItemDialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_CANCEL:
      if (interruptable::Cancel())
      {
        wxLogStatus(_("Cancelled"));
      }
      break;

    case wxID_OK:
    case wxID_APPLY:
      switch (dialogid)
      {
        case wxID_ADD:
          if (GetProject() != nullptr)
          {
            long flags = 0;
          
            if (config(GetProject()->GetTextAddFiles()).get(true)) flags |= dir::FILES;
            if (config(GetProject()->GetTextAddRecursive()).get(true)) flags |= dir::RECURSIVE;
            if (config(GetProject()->GetTextAddFolders()).get(true)) flags |= dir::DIRS;

            GetProject()->AddItems(
              config(GetProject()->GetTextInFolder()).firstof(),
              config(GetProject()->GetTextAddWhat()).firstof(),
              flags);
          }
          break;

        case ID_FIND_IN_FILES:
        case ID_REPLACE_IN_FILES:
          FindInFiles(dialogid);
          break;

        default: wxFAIL;
      }
      break;

    default: wxFAIL;
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
  
  auto* stc = GetSTC();
  auto* project = GetProject();

  if (const size_t pos = title.size() - indicator.size();
    (project != nullptr && project->GetContentsChanged()) ||
     // using GetContentsChanged gives assert in vcs dialog
    (stc != nullptr && stc->GetModify() && 
      (!(stc->GetData().Flags() & stc_data::WIN_NO_INDICATOR))))
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

void wex::history_frame::SetRecentFile(const wex::path& path)
{
  managed_frame::SetRecentFile(path);
  
  if (m_FileHistoryList != nullptr && path.FileExists())
  {
    listitem(m_FileHistoryList, path).Insert(0);

    if (m_FileHistoryList->GetItemCount() > 1)
    {
      for (auto i = m_FileHistoryList->GetItemCount() - 1; i >= 1 ; i--)
      {
        if (listitem item(m_FileHistoryList, i); item.GetFileName() == path)
        {
          item.Delete();
        }
      }
    }
  }
}

void wex::history_frame::UseFileHistoryList(listview* list)
{
  wxASSERT(list->GetData().Type() == listview_data::HISTORY);
  
  m_FileHistoryList = list;
  m_FileHistoryList->Hide();

  // Add all (existing) items from FileHistory.
  for (size_t i = 0; i < GetFileHistory().GetCount(); i++)
  {
    if (listitem item(m_FileHistoryList, 
      GetFileHistory().GetHistoryFile(i));
      item.GetFileName().GetStat().is_ok())
    {
      item.Insert();
    }
  }
}
