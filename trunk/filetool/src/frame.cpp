/******************************************************************************\
* File:          frame.cpp
* Purpose:       Implementation of exFrameWithHistory class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/stdpaths.h>
#include <wx/extension/util.h>
#include <wx/filetool/frame.h>
#include <wx/filetool/defs.h>
#include <wx/filetool/listitem.h>
#include <wx/filetool/listview.h>
#include <wx/filetool/stc.h>
#include <wx/filetool/util.h>

BEGIN_EVENT_TABLE(exFrameWithHistory, exManagedFrame)
  EVT_CLOSE(exFrameWithHistory::OnClose)
  EVT_IDLE(exFrameWithHistory::OnIdle)
  EVT_MENU(wxID_OPEN, exFrameWithHistory::OnCommand)
  EVT_MENU_RANGE(ID_FILETOOL_LOWEST, ID_FILETOOL_HIGHEST, exFrameWithHistory::OnCommand)
  EVT_UPDATE_UI(ID_VIEW_STATUSBAR, exFrameWithHistory::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_TOOLBAR, exFrameWithHistory::OnUpdateUI)
END_EVENT_TABLE()

exFrameWithHistory::exFrameWithHistory(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  size_t maxProjects,
  const wxString& project_wildcard,
  int style)
  : exManagedFrame(parent, id, title, style)
  , m_FileHistory(maxFiles, ID_RECENT_FILE_LOWEST)
  , m_FileHistoryList(NULL)
  , m_ProjectHistory(maxProjects, ID_RECENT_PROJECT_LOWEST)
  , m_ProjectWildcard(project_wildcard)
{
  // There is only support for one history in the config.
  // We use file history for this, so update project history ourselves.
  // The order should be inverted, as the last one added is the most recent used.
  if (maxProjects > 0)
  {
    for (int i = maxProjects - 1 ; i >=0 ; i--)
    {
      SetRecentProject(exApp::GetConfig(wxString::Format("RecentProject%d", i)));
    }
  }

#ifdef USE_EMBEDDED_SQL
  exTool::AddInfo(
    ID_TOOL_SQL,
    _("Executed %ld SQL queries in"),
    exEllipsed(_("&SQL Query Run")));

  exTool::AddInfo(
    ID_TOOL_REPORT_SQL,
    _("Reported %ld SQL queries in"),
    _("SQL &Query Report"),
    false);
#endif
}

exListViewFile* exFrameWithHistory::Activate(
  int WXUNUSED(type),
  const exLexer* WXUNUSED(lexer))
{
  return NULL;
}

bool exFrameWithHistory::DialogFileOpen(
  long style,
  const wxString wildcards,
  bool ask_for_continue)
{
  exSTCWithFrame* stc = GetCurrentSTC();

  wxString use_wildcards = wildcards;

  if (stc != NULL && use_wildcards.empty())
  {
    use_wildcards = exApp::GetLexers()->BuildWildCards(stc->GetFileName());
  }

  wxFileDialog dlg(this,
    _("Select Files"),
    wxEmptyString,
    wxEmptyString,
    use_wildcards,
    style);

  if (stc != NULL)
  {
    if (stc->AskFileOpen(dlg, ask_for_continue) == wxID_CANCEL) return false;
  }
  else
  {
    if (dlg.ShowModal() == wxID_CANCEL) return false;
  }

  wxArrayString files;
  dlg.GetPaths(files);
  exOpenFiles(this, files);

  return true;
}

bool exFrameWithHistory::DialogProjectOpen(long style)
{
  wxFileDialog dlg(this,
    _("Select Projects"),
    wxStandardPaths::Get().GetUserDataDir(),
    wxEmptyString,
    m_ProjectWildcard,
    style);

  if (dlg.ShowModal() == wxID_CANCEL) return false;

  wxArrayString files;
  dlg.GetPaths(files);
  exOpenFiles(this, files, exSTCWithFrame::STC_OPEN_IS_PROJECT);

  return true;
}

void exFrameWithHistory::DoRecent(wxFileHistory& history, int index, long flags)
{
  if (history.GetCount() > 0)
  {
    const wxString file(history.GetHistoryFile(index));

    if (!wxFileExists(file))
    {
      history.RemoveFileFromHistory(index);
      wxLogMessage(_("Removed not existing file: %s from history"), file.c_str());
    }
    else
    {
      OpenFile(file, 0, wxEmptyString, flags);
    }
  }
}

exSTCWithFrame* exFrameWithHistory::GetCurrentSTC()
{
  exSTC* stc = GetFocusedSTC();
  if (stc == NULL) return NULL;
  return wxDynamicCast(stc, exSTCWithFrame);
}

exListViewFile* exFrameWithHistory::GetFocusedListView()
{
  wxWindow* win = wxWindow::FindFocus();

  if (win == NULL)
  {
    return NULL;
  }

  return wxDynamicCast(win, exListViewFile);
}

void exFrameWithHistory::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    if (exListViewFile::ProcessIsRunning())
    {
      wxLogMessage(_("Process is running"));
      event.Veto();
      return;
    }
  }

  exListViewFile::CleanUp();
#ifdef EMBEDDED_SQL
  exTextFileWithReport::CleanUp();
#endif

  m_FileHistory.Save(*exApp::GetConfig());

  for (size_t i = 0; i < m_ProjectHistory.GetCount(); i++)
  {
    exApp::SetConfig(
      wxString::Format("RecentProject%d", i),
      m_ProjectHistory.GetHistoryFile(i));
  }

  event.Skip();
}

void exFrameWithHistory::OnCommand(wxCommandEvent& event)
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
      exSTCWithFrame::STC_OPEN_IS_PROJECT);
  }
  else
  {
    switch (event.GetId())
    {
    case wxID_OPEN:
      DialogFileOpen();
      break;

    case ID_PROJECT_OPEN:
      DialogProjectOpen();
      break;

    case ID_PROJECT_SAVE:
      {
        exListViewFile* project = GetCurrentProject();
        if (project != NULL && project->FileSave())
        {
          SetTitle(wxEmptyString, project->GetFileName().GetName());
        }
      }
      break;

    case ID_SPECIAL_FIND_IN_FILES:
      exFindInFiles(this);
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

void exFrameWithHistory::OnIdle(wxIdleEvent& event)
{
  event.Skip();

  wxWindow* win = wxWindow::FindFocus();
  if (win == NULL) return;

  exSTCWithFrame* editor = wxDynamicCast(win, exSTCWithFrame);
  exListViewFile* project = wxDynamicCast(win, exListViewFile);

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

void exFrameWithHistory::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
  case ID_VIEW_STATUSBAR:
    if (GetStatusBar() == NULL) return;
    event.Check(GetStatusBar()->IsShown());
    break;

  case ID_VIEW_TOOLBAR:
    if (GetToolBar() == NULL) return;
    event.Check(GetToolBar()->IsShown());
    break;

  default:
    wxFAIL;
  }
}

bool exFrameWithHistory::OpenFile(
  const exFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  if (exFrame::OpenFile(filename, line_number, match, flags))
  {
    SetRecentFile(filename.GetFullPath());
    return true;
  }

  return false;
}

void exFrameWithHistory::SetRecentFile(const wxString& file)
{
  if (!file.empty())
  {
    m_FileHistory.AddFileToHistory(file);

    if (m_FileHistoryList != NULL)
    {
      exListItemWithFileName item(m_FileHistoryList, file);
      item.Insert((long)0);

      if (m_FileHistoryList->GetItemCount() > 1)
      {
        for (int i = m_FileHistoryList->GetItemCount() - 1; i >= 1 ; i--)
        {
          exListItemWithFileName item(m_FileHistoryList, i);

          if (item.GetFileName().GetFullPath() == file)
          {
            m_FileHistoryList->DeleteItem(i);
          }
        }
      }
    }
  }
}

void exFrameWithHistory::SetTitle(const wxString& file, const wxString& project)
{
  // If one of the strings is empty, try to get a better string.
  wxString better_file(file);
  wxString better_project(project);

  if (better_file.empty())
  {
    exSTCWithFrame* stc = GetCurrentSTC();

    if (stc != NULL)
    {
      better_file = stc->GetFileName().GetFullPath();
    }
  }

  if (better_project.empty())
  {
    exListViewFile* lv = GetCurrentListView();

    if (lv != NULL && lv->GetType() == exListViewFile::LIST_PROJECT)
    {
      better_project = lv->GetFileName().GetName();
    }
  }

  // And now update the title.
  if (better_file.empty() && better_project.empty())
  {
    exFrame::SetTitle(wxTheApp->GetAppName());
  }
  else
  {
    const wxString sep =
      (!better_file.empty() && !better_project.empty() ?
          " - ":
          wxString(wxEmptyString));

    exFrame::SetTitle(better_file + sep + better_project);
  }
}

void exFrameWithHistory::UseFileHistory(wxWindowID id, wxMenu* menu)
{
  UseHistory(id, menu, m_FileHistory);

  // We can load file history now.
  m_FileHistory.Load(*exApp::GetConfig());
}

void exFrameWithHistory::UseFileHistoryList(exListViewFile* list)
{
  m_FileHistoryList = list;

  // Add all items from FileHistory.
  for (size_t i = 0; i < m_FileHistory.GetCount(); i++)
  {
    const wxString file = m_FileHistory.GetHistoryFile(i);
    exListItemWithFileName item(m_FileHistoryList, file);
    item.Insert();
  }
}

void exFrameWithHistory::UseHistory(wxWindowID id, wxMenu* menu, wxFileHistory& history)
{
  wxMenu* submenu = new wxMenu;
  menu->Append(id, _("Open &Recent"), submenu);
  history.UseMenu(submenu);
}

void exFrameWithHistory::UseProjectHistory(wxWindowID id, wxMenu* menu)
{
  UseHistory(id, menu, m_ProjectHistory);

  // And add the files to the menu.
  m_ProjectHistory.AddFilesToMenu();
}
