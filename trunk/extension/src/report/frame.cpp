/******************************************************************************\
* File:          frame.cpp
* Purpose:       Implementation of wxExFrameWithHistory class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/tokenzr.h> 
#include <wx/extension/configdlg.h>
#include <wx/extension/frd.h>
#include <wx/extension/log.h>
#include <wx/extension/util.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/listitem.h>
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
  EVT_MENU(wxID_OPEN, wxExFrameWithHistory::OnCommand)
  EVT_MENU(wxID_PREFERENCES, wxExFrameWithHistory::OnCommand)
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
  , m_TextInFiles(_("In files"))
  , m_TextInFolder(_("In folder"))
  , m_FileHistory(maxFiles, wxID_FILE1)
  , m_FileHistoryList(NULL)
  , m_ProjectHistory(maxProjects, ID_RECENT_PROJECT_LOWEST)
  , m_Process(NULL)
{
  // There is only support for one history in the config.
  // We use file history for this, so update project history ourselves.
  // The order should be inverted, as the last one added is the most recent used.
  for (int i = m_ProjectHistory.GetMaxFiles() - 1 ; i >=0 ; i--)
  {
    SetRecentProject(
      wxConfigBase::Get()->Read(wxString::Format("RecentProject%d", i)));
  }

#ifdef wxExUSE_EMBEDDED_SQL
  wxExTool::Get()->AddInfo(
    ID_TOOL_SQL,
    _("Executed %ld SQL queries in"),
    wxExEllipsed(_("&SQL Query Run")));

  wxExTool::Get()->AddInfo(
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
}

void wxExFrameWithHistory::DoRecent(
  wxFileHistory& history, 
  int index, 
  long flags)
{
  if (history.GetCount() > 0)
  {
    const wxString file(history.GetHistoryFile(index));

    if (!wxFileExists(file))
    {
      history.RemoveFileFromHistory(index);
      wxLogMessage(_("Removed not existing file: %s from history"), 
        file.c_str());
    }
    else
    {
      OpenFile(file, 0, wxEmptyString, flags);
    }
  }
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

  wxExLog::Get()->Log(
    wxExFindReplaceData::Get()->GetFindReplaceInfoText(replace));

  wxExDirTool dir(
    tool,
    wxExConfigFirstOf(m_TextInFolder),
    wxExConfigFirstOf(m_TextInFiles));

  dir.FindFiles();

  tool.Log(
    &dir.GetStatistics().GetElements(), 
    wxExConfigFirstOf(m_TextInFolder));
}

int wxExFrameWithHistory::FindInFilesDialog(int id)
{
  if (m_FiFDialog != NULL)
  {
    m_FiFDialog->Destroy();
    m_FiFDialog = NULL;
  }

  GetSearchText();

  std::vector<wxExConfigItem> v;
  v.push_back(
    wxExConfigItem(wxExFindReplaceData::Get()->GetTextFindWhat(), 
    CONFIG_COMBOBOX, 
    wxEmptyString, 
    true));

  if (id == ID_REPLACE_IN_FILES) 
  {
    v.push_back(wxExConfigItem(
      wxExFindReplaceData::Get()->GetTextReplaceWith(), 
      CONFIG_COMBOBOX));
  }
  
  v.push_back(wxExConfigItem(
    m_TextInFiles, 
    CONFIG_COMBOBOX, 
    wxEmptyString, 
    true));

  v.push_back(wxExConfigItem(
    m_TextInFolder, 
    CONFIG_COMBOBOXDIR, 
    wxEmptyString, 
    true,
    1000)); // TODO: fix

  v.push_back(wxExConfigItem());

  if (id == ID_REPLACE_IN_FILES) 
  {
    // Match whole word does not work with replace.
    std::set<wxString> s;
    s.insert(wxExFindReplaceData::Get()->GetTextMatchCase());
    s.insert(wxExFindReplaceData::Get()->GetTextRegEx());
    v.push_back(wxExConfigItem(s));
  }
  else
  {
    v.push_back(wxExConfigItem(wxExFindReplaceData::Get()->GetInfo()));
  }

  m_FiFDialog = new wxExConfigDialog(this,
    v,
    (id == ID_REPLACE_IN_FILES ? _("Replace In Files"): _("Find In Files")),
    0,
    2,
    wxOK | wxCANCEL,
    id);
  
  return m_FiFDialog->Show();
}

void wxExFrameWithHistory::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    if (wxExProcess::Get()->IsRunning())
    {
      wxLogMessage(_("Process is running"));
      event.Veto();
      return;
    }
  }

  wxDELETE(m_Process);

  delete wxExProcess::Set(NULL);

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
      wxExSTCWithFrame::STC_WIN_IS_PROJECT);
  }
  else
  {
    switch (event.GetId())
    {
    case wxID_OPEN:
      if (!event.GetString().empty())
      {
        wxArrayString files;
        wxStringTokenizer tkz(event.GetString());
        wxExSTCFile* stc = GetSTC();

        while (tkz.HasMoreTokens())
        {
          const wxString token = tkz.GetNextToken();

          wxFileName file(token);
  
          if (file.IsRelative() && stc != NULL)
          {
            file.MakeAbsolute(stc->GetFileName().GetPath());

            if (!file.FileExists())
            {
              wxLogError(_("Cannot locate file") + ": " + token);
            }
            else
            {
              files.Add(file.GetFullPath());
            }
          }
          else
          {
            files.Add(file.GetFullPath());
          }
        }

        wxExOpenFiles(this, files);
      }
      else
      {
        wxExOpenFilesDialog(this);
      }
      break;
      
    case wxID_PREFERENCES:
      wxExSTCFile::ConfigDialog(this,
        _("Editor Options"),
        wxExSTCFile::STC_CONFIG_MODELESS | 
        wxExSTCFile::STC_CONFIG_SIMPLE |
        wxExSTCFile::STC_CONFIG_WITH_APPLY,
        event.GetId());
    break;

    case ID_PROJECT_SAVE:
      {
        wxExListViewFile* project = GetProject();
        if (project != NULL)
        {
          project->FileSave();
          SetTitle(wxEmptyString, project->GetFileName().GetName());
        }
      }
      break;

    case ID_FIND_IN_FILES:
    case ID_REPLACE_IN_FILES:
      FindInFilesDialog(event.GetId());
      break;

    case ID_TERMINATED_PROCESS:
      wxBell();
      wxDELETE(m_Process);
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
#if wxUSE_STATUSBAR
        wxExFrame::StatusText(_("Cancelled"));
#endif
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

  wxExSTCFile* stc = GetFocusedSTC();
  wxExListViewFile* project = GetProject();

  const wxString title(GetTitle());
  const wxUniChar indicator('*');

  if ((project != NULL && project->GetContentsChanged()) ||
       // using GetContentsChanged gives assert in vcs dialog
      (stc != NULL && stc->GetModify()))
  {
    // Project or editor changed, add indicator if not yet done.
    if (title.Last() != indicator)
    {
      wxFrame::SetTitle(title + " " + indicator);
    }
  }
  else
  {
    // Project or editor not changed, remove indicator if not yet done.
    if (title.Last() == indicator)
    {
      wxFrame::SetTitle(title.substr(0, title.length() - 2));
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

bool wxExFrameWithHistory::ProcessRun(const wxString& command)
{
  wxASSERT(m_Process == NULL);

  if ((m_Process = new wxExProcess(this, command)) != NULL)
  {
    if (m_Process->Execute() > 0)
    {
      return true;
    }

    wxDELETE(m_Process);
  }

  return false;
}

bool wxExFrameWithHistory::ProcessStop()
{
  if (wxExProcess::Get()->IsRunning())
  {
    if (m_Process->Kill() == wxKILL_ERROR)
    {
      // Even if the process could not be killed, set it to NULL, as it is deleted.
      wxFAIL;
      m_Process = NULL;
      return false;
    }
    else
    {
      m_Process = NULL;
      return true;
    }
  }

  return true;
}

void wxExFrameWithHistory::SetRecentFile(const wxString& file)
{
  if (!file.empty())
  {
    m_FileHistory.AddFileToHistory(file);

    if (m_FileHistoryList != NULL)
    {
      wxExListItem item(m_FileHistoryList, file);
      item.Insert((long)0);

      if (m_FileHistoryList->GetItemCount() > 1)
      {
        for (int i = m_FileHistoryList->GetItemCount() - 1; i >= 1 ; i--)
        {
          wxExListItem item(m_FileHistoryList, i);

          if (item.GetFileName().GetFullPath() == file)
          {
            m_FileHistoryList->DeleteItem(i);
          }
        }
      }
    }
  }
}

void wxExFrameWithHistory::SetTitle(
  const wxString& file, 
  const wxString& project)
{
  // If one of the strings is empty, try to get a better string.
  wxString better_file(file);
  wxString better_project(project);

  if (better_file.empty())
  {
    wxExSTCFile* stc = GetSTC();

    if (stc != NULL)
    {
      better_file = stc->GetFileName().GetFullPath();
    }
  }

  if (better_project.empty())
  {
    wxExListViewFile* lv = (wxExListViewFile*)GetListView();

    if (lv != NULL && lv->GetType() == wxExListViewStandard::LIST_FILE)
    {
      better_project = lv->GetFileName().GetName();
    }
  }

  // And now update the title.
  if (better_file.empty() && better_project.empty())
  {
    wxExFrame::SetTitle(wxTheApp->GetAppName());
  }
  else
  {
    const wxString sep =
      (!better_file.empty() && !better_project.empty() ?
          wxString(" - "):
          wxString(wxEmptyString));

    wxExFrame::SetTitle(better_file + sep + better_project);
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
  UseHistory(id, menu, m_ProjectHistory);

  // And add the files to the menu.
  m_ProjectHistory.AddFilesToMenu();
}
