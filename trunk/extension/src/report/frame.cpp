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

#include <wx/stdpaths.h>
#include <wx/extension/report/report.h>

BEGIN_EVENT_TABLE(wxExFrameWithHistory, wxExManagedFrame)
  EVT_CLOSE(wxExFrameWithHistory::OnClose)
  EVT_IDLE(wxExFrameWithHistory::OnIdle)
  EVT_MENU(wxID_OPEN, wxExFrameWithHistory::OnCommand)
  EVT_MENU(wxID_PREFERENCES, wxExFrameWithHistory::OnCommand)
  EVT_MENU(ID_TERMINATED_PROCESS, wxExFrameWithHistory::OnCommand)
  EVT_MENU_RANGE(
    ID_EXTENSION_REPORT_LOWEST, 
    ID_EXTENSION_REPORT_HIGHEST, 
    wxExFrameWithHistory::OnCommand)
  EVT_UPDATE_UI(ID_VIEW_STATUSBAR, wxExFrameWithHistory::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_TOOLBAR, wxExFrameWithHistory::OnUpdateUI)
END_EVENT_TABLE()

wxExFrameWithHistory::wxExFrameWithHistory(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  size_t maxProjects,
  int style)
  : wxExManagedFrame(parent, id, title, style)
  , m_FileHistory(maxFiles, ID_RECENT_FILE_LOWEST)
  , m_FileHistoryList(NULL)
  , m_ProjectHistory(maxProjects, ID_RECENT_PROJECT_LOWEST)
  , m_Process(NULL)
{
  // There is only support for one history in the config.
  // We use file history for this, so update project history ourselves.
  // The order should be inverted, as the last one added is the most recent used.
  if (maxProjects > 0)
  {
    for (int i = maxProjects - 1 ; i >=0 ; i--)
    {
      SetRecentProject(
        wxConfigBase::Get()->Read(wxString::Format("RecentProject%d", i)));
    }
  }

#ifdef wxExUSE_EMBEDDED_SQL
  wxExTool::AddInfo(
    ID_TOOL_SQL,
    _("Executed %ld SQL queries in"),
    wxExEllipsed(_("&SQL Query Run")));

  wxExTool::AddInfo(
    ID_TOOL_REPORT_SQL,
    _("Reported %ld SQL queries in"),
    _("Report SQL &Query"));
#endif
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

wxExListViewWithFrame* wxExFrameWithHistory::GetProject()
{
  return (wxExListViewWithFrame*)GetFocusedListView();
}

void wxExFrameWithHistory::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    if (ProcessIsRunning())
    {
      wxLogMessage(_("Process is running"));
      event.Veto();
      return;
    }
  }

  wxDELETE(m_Process);

  m_FileHistory.Save(*wxConfigBase::Get());

  for (size_t i = 0; i < m_ProjectHistory.GetCount(); i++)
  {
    wxExApp::SetConfig(
      wxString::Format("RecentProject%d", i),
      m_ProjectHistory.GetHistoryFile(i));
  }

  event.Skip();
}

void wxExFrameWithHistory::OnCommand(wxCommandEvent& event)
{
  if (event.GetId() >= ID_RECENT_FILE_LOWEST &&
      event.GetId() <= ID_RECENT_FILE_HIGHEST)
  {
    DoRecent(m_FileHistory,
      event.GetId() - ID_RECENT_FILE_LOWEST);
  }
  else if (event.GetId() >= ID_RECENT_PROJECT_LOWEST &&
           event.GetId() <= ID_RECENT_PROJECT_HIGHEST)
  {
    DoRecent(m_ProjectHistory,
      event.GetId() - ID_RECENT_PROJECT_LOWEST,
      wxExSTCWithFrame::STC_OPEN_IS_PROJECT);
  }
  else
  {
    switch (event.GetId())
    {
    case wxID_OPEN:
      wxExOpenFilesDialog(this);
      break;
      
    case wxID_PREFERENCES:
      wxExSTC::ConfigDialog(this,
        _("Editor Options"),
        wxExSTC::STC_CONFIG_MODELESS | 
        wxExSTC::STC_CONFIG_SIMPLE |
        wxExSTC::STC_CONFIG_WITH_APPLY,
        event.GetId());
    break;

    case ID_PROJECT_SAVE:
      {
        wxExListViewWithFrame* project = GetProject();
        if (project != NULL)
        {
          project->FileSave();
          SetTitle(wxEmptyString, project->GetFileName().GetName());
        }
      }
      break;

    case ID_SPECIAL_FIND_IN_FILES:
      wxExFindInFiles();
      break;

    case ID_SPECIAL_REPLACE_IN_FILES:
      wxExFindInFiles(false);
      break;

    case ID_TERMINATED_PROCESS:
      wxBell();
      wxDELETE(m_Process);
    break;

    case ID_VIEW_STATUSBAR:
      GetStatusBar()->Show(!GetStatusBar()->IsShown());
      SendSizeEvent();
      break;

    case ID_VIEW_TOOLBAR:
      GetToolBar()->Show(!GetToolBar()->IsShown());
      SendSizeEvent();
      break;

    default:
      wxFAIL;
    }
  }
}

void wxExFrameWithHistory::OnIdle(wxIdleEvent& event)
{
  event.Skip();

  wxWindow* win = wxWindow::FindFocus();
  if (win == NULL) return;

  wxExSTCWithFrame* editor = wxDynamicCast(win, wxExSTCWithFrame);
  wxExListViewWithFrame* project = wxDynamicCast(win, wxExListViewWithFrame);

  const wxString title(GetTitle());
  const wxChar indicator('*');

  if ((project != NULL && project->GetContentsChanged()) ||
      (editor != NULL && editor->GetContentsChanged()))
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

void wxExFrameWithHistory::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
  case ID_VIEW_STATUSBAR:
    wxASSERT(GetStatusBar() != NULL);
    event.Check(GetStatusBar()->IsShown());
    break;

  case ID_VIEW_TOOLBAR:
    wxASSERT(GetToolBar() != NULL) ;
    event.Check(GetToolBar()->IsShown());
    break;

  default:
    wxFAIL;
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

bool wxExFrameWithHistory::ProcessIsRunning() const
{
  return m_Process != NULL && wxProcess::Exists(m_Process->GetPid());
}

bool wxExFrameWithHistory::ProcessRun(const wxString& command)
{
  wxASSERT(m_Process == NULL);

  wxExListViewWithFrame* listview = Activate(wxExListViewWithFrame::LIST_PROCESS);

  if (listview == NULL) 
  {
    wxFAIL;
    return false;
  }

  if ((m_Process = new wxExProcessWithListView(this, listview, command)) != NULL)
  {
    if (m_Process->Execute() <= 0)
    {
      wxDELETE(m_Process);
      return false;
    }
    else
    {
      return true;
    }
  }

  return false;
}

bool wxExFrameWithHistory::ProcessStop()
{
  if (ProcessIsRunning())
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
      wxExListItemWithFileName item(m_FileHistoryList, file);
      item.Insert((long)0);

      if (m_FileHistoryList->GetItemCount() > 1)
      {
        for (int i = m_FileHistoryList->GetItemCount() - 1; i >= 1 ; i--)
        {
          wxExListItemWithFileName item(m_FileHistoryList, i);

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
    wxExSTC* stc = GetSTC();

    if (stc != NULL)
    {
      better_file = stc->GetFileName().GetFullPath();
    }
  }

  if (better_project.empty())
  {
    wxExListViewWithFrame* lv = (wxExListViewWithFrame*)GetListView();

    if (lv != NULL && lv->GetType() == wxExListViewWithFrame::LIST_PROJECT)
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
          " - ":
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

void wxExFrameWithHistory::UseFileHistoryList(wxExListViewWithFrame* list)
{
  m_FileHistoryList = list;
  m_FileHistoryList->Hide();

  // Add all items from FileHistory.
  for (size_t i = 0; i < m_FileHistory.GetCount(); i++)
  {
    wxExListItemWithFileName item(
      m_FileHistoryList, 
      m_FileHistory.GetHistoryFile(i));

    item.Insert();
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
