////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wxExFrameWithHistory class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frd.h>
#include <wx/extension/listitem.h>
#include <wx/extension/util.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/process.h>
#include <wx/extension/report/stc.h>
#include <wx/extension/report/util.h>

// The maximal number of files and projects to be supported.
const int NUMBER_RECENT_FILES = 25;
const int NUMBER_RECENT_PROJECTS = 25;
const int ID_RECENT_PROJECT_LOWEST =  wxID_FILE1 + NUMBER_RECENT_FILES + 1;

BEGIN_EVENT_TABLE(wxExFrameWithHistory, wxExManagedFrame)
  EVT_CLOSE(wxExFrameWithHistory::OnClose)
  EVT_IDLE(wxExFrameWithHistory::OnIdle)
  EVT_MENU(ID_TERMINATED_PROCESS, wxExFrameWithHistory::OnCommand)
  EVT_MENU_RANGE(
    wxID_FILE1, 
    wxID_FILE1 + NUMBER_RECENT_FILES, wxExFrameWithHistory::OnCommand)
  EVT_MENU_RANGE(
    ID_RECENT_PROJECT_LOWEST, 
    ID_RECENT_PROJECT_LOWEST + NUMBER_RECENT_PROJECTS, wxExFrameWithHistory::OnCommand)
  EVT_MENU_RANGE(
    ID_EXTENSION_REPORT_LOWEST, 
    ID_EXTENSION_REPORT_HIGHEST, 
    wxExFrameWithHistory::OnCommand)
END_EVENT_TABLE()

wxExFrameWithHistory::wxExFrameWithHistory(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  size_t maxProjects,
  int style)
  : wxExManagedFrame(parent, id, title, style)
  , m_FiFDialog(NULL)
  , m_RiFDialog(NULL)
  , m_TextInFiles(_("In files"))
  , m_TextInFolder(_("In folder"))
  , m_TextRecursive(_("Recursive"))
  , m_FileHistory(maxFiles, wxID_FILE1)
  , m_FileHistoryList(NULL)
  , m_ProjectHistory(maxProjects, ID_RECENT_PROJECT_LOWEST)
  , m_Process(new wxExProcessListView(this))
{
  // There is only support for one history in the config.
  // We use file history for this, so update project history ourselves.
  // The order should be inverted, as the last one added is the most recent used.
  for (int i = m_ProjectHistory.GetMaxFiles() - 1 ; i >=0 ; i--)
  {
    SetRecentProject(
      wxConfigBase::Get()->Read(wxString::Format("RecentProject%d", i)));
  }

  // Take care of default value.
  if (!wxConfigBase::Get()->Exists(m_TextRecursive))
  {
    wxConfigBase::Get()->Write(m_TextRecursive, true); 
  }
  
#ifdef wxExUSE_EMBEDDED_SQL
  wxExTool().AddInfo(
    ID_TOOL_SQL,
    _("Executed %ld SQL queries in"),
    wxExEllipsed(_("&SQL Query Run")));

  wxExTool().AddInfo(
    ID_TOOL_REPORT_SQL,
    _("Reported %ld SQL queries in"),
    _("Report SQL &Query"));
#endif
}

wxExFrameWithHistory::~wxExFrameWithHistory()
{
  if (m_FiFDialog != NULL)
  {
    m_FiFDialog->Destroy();
  }
  
  if (m_RiFDialog != NULL)
  {
    m_RiFDialog->Destroy();
  }
  
  wxDELETE(m_Process);
}

void wxExFrameWithHistory::CreateDialogs()
{
  std::vector<wxExConfigItem> f;
  std::vector<wxExConfigItem> r;

  f.push_back(
    wxExConfigItem(wxExFindReplaceData::Get()->GetTextFindWhat(), 
    CONFIG_COMBOBOX, 
    wxEmptyString, 
    true));
  r.push_back(f.back());

  r.push_back(wxExConfigItem(
    wxExFindReplaceData::Get()->GetTextReplaceWith(), 
    CONFIG_COMBOBOX));
  
  f.push_back(wxExConfigItem(
    m_TextInFiles, 
    CONFIG_COMBOBOX, 
    wxEmptyString, 
    true));
  r.push_back(f.back());

  f.push_back(wxExConfigItem(
    m_TextInFolder, 
    CONFIG_COMBOBOXDIR, 
    wxEmptyString, 
    true,
    1000));
  r.push_back(f.back());

  // Match whole word does not work with replace.
  std::set<wxString> s;
  s.insert(wxExFindReplaceData::Get()->GetTextMatchCase());
  s.insert(wxExFindReplaceData::Get()->GetTextRegEx());
  s.insert(m_TextRecursive);
  r.push_back(wxExConfigItem(s));
  
  std::set<wxString> t(wxExFindReplaceData::Get()->GetInfo());
  t.insert(m_TextRecursive);
  f.push_back(wxExConfigItem(t));
  
  m_FiFDialog = new wxExConfigDialog(this,
    f,
    _("Find In Files"),
    0,
    1,
    wxOK | wxCANCEL,
    ID_FIND_IN_FILES);
    
  m_RiFDialog = new wxExConfigDialog(this,
    r,
    _("Replace In Files"),
    0,
    1,
    wxOK | wxCANCEL,
    ID_REPLACE_IN_FILES);
}

void wxExFrameWithHistory::DoRecent(
  wxFileHistory& history, 
  size_t index, 
  long flags)
{
  if (history.GetCount() > 0 && index < history.GetMaxFiles())
  {
    const wxString file(history.GetHistoryFile(index));

    if (!wxFileExists(file))
    {
      history.RemoveFileFromHistory(index);
      wxLogStatus(_("Removed not existing file: %s from history"), 
        file.c_str());
    }
    else
    {
      OpenFile(file, 0, wxEmptyString, flags);
    }
  }
}

void wxExFrameWithHistory::FileHistoryPopupMenu()
{
  wxMenu* menu = new wxMenu();

  for (int i = 0; i < m_FileHistory.GetCount(); i++)
  {
    const wxFileName file(m_FileHistory.GetHistoryFile(i));
    
    wxMenuItem* item = new wxMenuItem(
      menu, 
      wxID_FILE1 + i, 
      file.GetFullName());

    if (file.FileExists())
    {
      item->SetBitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
        wxExGetIconID(file)));
    }
    
    menu->Append(item);
  }
  
  if (menu->GetMenuItemCount() > 0)
  {
    PopupMenu(menu);
  }
    
  delete menu;
}
  
void wxExFrameWithHistory::FindInFiles(wxWindowID dialogid)
{
  const bool replace = (dialogid == ID_REPLACE_IN_FILES);
  const wxExTool tool =
    (replace ?
       ID_TOOL_REPORT_REPLACE:
       ID_TOOL_REPORT_FIND);

  if (!wxExTextFileWithListView::SetupTool(tool, this))
  {
    return;
  }

  wxLogStatus(
    wxExFindReplaceData::Get()->GetFindReplaceInfoText(replace));
    
  int flags = wxDIR_FILES | wxDIR_HIDDEN;
  
  if (wxConfigBase::Get()->ReadBool(m_TextRecursive, true)) 
  {
    flags |= wxDIR_DIRS;
  }

  wxExDirTool dir(
    tool,
    wxExConfigFirstOf(m_TextInFolder),
    wxExConfigFirstOf(m_TextInFiles),
    flags);

  dir.FindFiles();

  tool.Log(&dir.GetStatistics().GetElements());
}

void wxExFrameWithHistory::FindInSelection(
  const wxArrayString& files,
  int id)
{
  const wxExFileName filename(files[0]);
  const wxExTool tool(id);
  
  if (FindInSelectionDialog(
    tool.GetId(),
    filename.DirExists() && !filename.FileExists()) == wxID_CANCEL ||
     !wxExTextFileWithListView::SetupTool(tool, this))
  {
    return;
  }
  
  wxExStatistics<long> stats;

  for (auto i = 0; i < files.GetCount(); i++)
  {
    const wxExFileName fn(files[i]);
    
    if (fn.FileExists())
    {
      wxExTextFileWithListView file(fn, tool);
      file.RunTool();
      stats += file.GetStatistics().GetElements();
    }
    else
    {
      wxExDirTool dir(
        tool, 
        fn.GetFullPath(), 
        wxExConfigFirstOf(m_TextInFiles));
        
      dir.FindFiles();
      stats += dir.GetStatistics().GetElements();
    }
  }
  
  tool.Log(&stats);
}

int wxExFrameWithHistory::FindInSelectionDialog(
  int id,
  bool add_in_files)
{
  if (GetSTC() != NULL)
  {
    GetSTC()->GetFindString();
  }

  std::vector<wxExConfigItem> v;

  v.push_back(wxExConfigItem(
    wxExFindReplaceData::Get()->GetTextFindWhat(), 
    CONFIG_COMBOBOX, 
    wxEmptyString, 
    true));

  if (add_in_files)
  {
    v.push_back(wxExConfigItem(
      m_TextInFiles, 
      CONFIG_COMBOBOX, 
      wxEmptyString, 
      true));
  }
    
  if (id == ID_TOOL_REPORT_REPLACE) 
  {
    v.push_back(wxExConfigItem(
      wxExFindReplaceData::Get()->GetTextReplaceWith(), 
      CONFIG_COMBOBOX));
  }

  v.push_back(wxExConfigItem(wxExFindReplaceData::Get()->GetInfo()));

  if (wxExConfigDialog(this,
    v,
    GetFindInCaption(id)).ShowModal() == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  wxLogStatus(
    wxExFindReplaceData::Get()->GetFindReplaceInfoText(
      id == ID_TOOL_REPORT_REPLACE));
        
  return wxID_OK;
}

const wxString wxExFrameWithHistory::GetFindInCaption(int id) const
{
  return (id == ID_TOOL_REPORT_REPLACE ?
    _("Replace In Selection"):
    _("Find In Selection"));
}

void wxExFrameWithHistory::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    if (m_Process->IsRunning())
    {
      wxLogStatus(_("Process is running"));
      event.Veto();
      return;
    }
  }

  m_FileHistory.Save(*wxConfigBase::Get());

  for (size_t i = 0; i < m_ProjectHistory.GetCount(); i++)
  {
    wxConfigBase::Get()->Write(
      wxString::Format("RecentProject%d", i),
      m_ProjectHistory.GetHistoryFile(i));
  }

  event.Skip();
}

void wxExFrameWithHistory::OnCommand(wxCommandEvent& event)
{
  if (event.GetId() >= wxID_FILE1 &&
      event.GetId() <= wxID_FILE1 + NUMBER_RECENT_FILES)
  {
    DoRecent(m_FileHistory,
      event.GetId() - wxID_FILE1);
  }
  else if (event.GetId() >= ID_RECENT_PROJECT_LOWEST &&
           event.GetId() <= ID_RECENT_PROJECT_LOWEST + NUMBER_RECENT_PROJECTS)
  {
    DoRecent(m_ProjectHistory,
      event.GetId() - ID_RECENT_PROJECT_LOWEST,
      WIN_IS_PROJECT);
  }
  else
  {
    switch (event.GetId())
    {
    case ID_FIND_IN_FILES: 
      if (m_FiFDialog == NULL)
      {
        CreateDialogs();
      }

      if (GetSTC() != NULL)
      {
        if (!GetSTC()->GetFindString().empty())
        {
          m_FiFDialog->Reload(); 
        }
      }
        
      m_FiFDialog->Show(); 
      break;
      
    case ID_REPLACE_IN_FILES: 
      if (m_RiFDialog == NULL)
      {
        CreateDialogs();
      }
      
      if (GetSTC() != NULL)
      {
        if (!GetSTC()->GetFindString().empty())
        {
          m_RiFDialog->Reload(); 
        }
      }
      
      m_RiFDialog->Show(); 
      break;

    case ID_PROJECT_SAVE:
      {
        wxExListViewFile* project = GetProject();
        
        if (project != NULL)
        {
          project->FileSave();
        }
      }
      break;
      
    case ID_TERMINATED_PROCESS:
      wxBell();
    break;

    default:
      wxFAIL;
    }
  }
}

void wxExFrameWithHistory::OnCommandConfigDialog(
  wxWindowID dialogid,
  int commandid)
{
  switch (commandid)
  {
    case wxID_CANCEL:
      if (wxExDir::GetIsBusy())
      {
        wxExDir::Cancel();
        wxLogStatus(_("Cancelled"));
      }
      break;

    case wxID_OK:
      switch (dialogid)
      {
        case wxID_ADD:
          GetProject()->AddItems();
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

void wxExFrameWithHistory::OnIdle(wxIdleEvent& event)
{
  event.Skip();

  const wxString title(GetTitle());
  
  if (title.empty())
  {
    return;
  }
  
  auto* stc = GetSTC();
  auto* project = GetProject();

  const wxUniChar indicator('*');

  if ((project != NULL && project->GetContentsChanged()) ||
       // using GetContentsChanged gives assert in vcs dialog
      (stc != NULL && stc->GetModify() && stc->GetName() != "SHELL"))
  {
    // Project or editor changed, add indicator if not yet done.
    if (title.Last() != indicator)
    {
      SetTitle(title + " " + indicator);
    }
  }
  else
  {
    // Project or editor not changed, remove indicator if not yet done.
    if (title.Last() == indicator && title.size() > 2)
    {
      SetTitle(title.substr(0, title.length() - 2));
    }
  }
}

bool wxExFrameWithHistory::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  if (wxExManagedFrame::OpenFile(filename, line_number, match, flags))
  {
    SetRecentFile(filename.GetFullPath());
    return true;
  }

  return false;
}

int wxExFrameWithHistory::ProcessConfigDialog(
  wxWindow* parent, 
  const wxString& title) const
{
  return wxExProcess::ConfigDialog(parent, title);
}

bool wxExFrameWithHistory::ProcessIsRunning() const
{
  return m_Process->IsRunning();
}
  
bool wxExFrameWithHistory::ProcessIsSelected() const
{
  return wxExProcess::IsSelected();
}
  
bool wxExFrameWithHistory::ProcessRun(const wxString& command, const wxString& wd)
{
  return m_Process->Execute(command, wd) > 0);
}

void wxExFrameWithHistory::ProcessStop()
{
  m_Process->Kill();
}

void wxExFrameWithHistory::SetRecentFile(const wxString& file)
{
  if (file.empty())
  {
    return;
  }
  
  m_FileHistory.AddFileToHistory(file);

  if (m_FileHistoryList != NULL)
  {
    wxExListItem item(m_FileHistoryList, file);
    item.Insert((long)0);

    if (m_FileHistoryList->GetItemCount() > 1)
    {
      for (auto i = m_FileHistoryList->GetItemCount() - 1; i >= 1 ; i--)
      {
        wxExListItem item(m_FileHistoryList, i);

        if (item.GetFileName().GetFullPath() == file)
        {
          item.Delete();
        }
      }
    }
  }
}

void wxExFrameWithHistory::SetRecentProject(const wxString& project) 
{
  if (!project.empty() && m_ProjectHistory.GetMaxFiles() > 0) 
  {
    m_ProjectHistory.AddFileToHistory(project);
  }
}
    
void wxExFrameWithHistory::UseFileHistory(wxWindowID id, wxMenu* menu)
{
  UseHistory(id, menu, m_FileHistory);

  // We can load file history now.
  m_FileHistory.Load(*wxConfigBase::Get());
}

void wxExFrameWithHistory::UseFileHistoryList(wxExListView* list)
{
  m_FileHistoryList = list;
  m_FileHistoryList->Hide();

  // Add all (existing) items from FileHistory.
  for (size_t i = 0; i < m_FileHistory.GetCount(); i++)
  {
    wxExListItem item(
      m_FileHistoryList, 
      m_FileHistory.GetHistoryFile(i));

    if (item.GetFileName().GetStat().IsOk())
    {
      item.Insert();
    }
  }
}

void wxExFrameWithHistory::UseHistory(
  wxWindowID id, 
  wxMenu* menu, 
  wxFileHistory& history)
{
  wxMenu* submenu = new wxMenu;
  menu->Append(id, _("Open &Recent"), submenu);
  history.UseMenu(submenu);
}

void wxExFrameWithHistory::UseProjectHistory(wxWindowID id, wxMenu* menu)
{
  if (m_ProjectHistory.GetMaxFiles() > 0)
  {
    UseHistory(id, menu, m_ProjectHistory);

    // And add the files to the menu.
    m_ProjectHistory.AddFilesToMenu();
  }
}
