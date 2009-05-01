/******************************************************************************\
* File:          frame.cpp
* Purpose:       Implementation of class 'MDIFrame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/aboutdlg.h>
#include <wx/numdlg.h>
#include <wx/stdpaths.h> // for wxStandardPaths
#include <wx/extension/configdialog.h>
#include <wx/extension/svn.h>
#include "frame.h"
#include "defs.h"
#include "version.h"

BEGIN_EVENT_TABLE(MDIFrame, Frame)
  EVT_BUTTON(wxID_STOP, MDIFrame::OnCommand)
  EVT_BUTTON(wxID_EXECUTE, MDIFrame::OnCommand)
  EVT_CLOSE(MDIFrame::OnClose)
  EVT_MENU_RANGE(wxID_LOWEST, wxID_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_APPL_LOWEST, ID_APPL_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_LOWEST, ID_EDIT_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_STC_LOWEST, ID_STC_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, MDIFrame::OnCommand)
#if wxUSE_CHECKBOX
  EVT_CHECKBOX(ID_EDIT_HEX_MODE, MDIFrame::OnCommand)
  EVT_CHECKBOX(ID_SYNC_MODE, MDIFrame::OnCommand)
#endif
  EVT_TREE_ITEM_ACTIVATED(wxID_TREECTRL, MDIFrame::OnTree)
  EVT_TREE_ITEM_RIGHT_CLICK(wxID_TREECTRL, MDIFrame::OnTree)
  EVT_UPDATE_UI(ID_ALL_STC_CLOSE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_ALL_STC_PRINT, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_ALL_STC_SAVE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_CLOSE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_COPY, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_CUT, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_EXECUTE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_JUMP_TO, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_FIND, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REPLACE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PRINT, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PREVIEW, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REPLACE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PASTE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UNDO, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REDO, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_STOP, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_CONTROL_CHAR, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_PLAYBACK, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_START_RECORD, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_STOP_RECORD, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_OPTION_LIST_SORT_TOGGLE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_PROJECT_SAVE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_FILE_MENU, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_PROJECT_MENU, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SORT_SYNC, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(wxID_SAVE, wxID_SAVEAS, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(wxID_SORT_ASCENDING, wxID_SORT_DESCENDING, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(wxID_VIEW_DETAILS, wxID_VIEW_LIST, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_EDIT_FIND_NEXT, ID_EDIT_FIND_PREVIOUS, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_EDIT_TOGGLE_FOLD, ID_EDIT_UNFOLD_ALL, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_PROJECT_OPENTEXT, ID_PROJECT_SAVEAS, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_VIEW_PANE_FIRST + 1, ID_VIEW_PANE_LAST - 1, MDIFrame::OnUpdateUI)
END_EVENT_TABLE()

const wxString project_wildcard = wxString(_("Project Files")) + " (*.prj)|*.prj";

MDIFrame::MDIFrame(bool open_recent)
  : Frame(project_wildcard)
  , m_NewFileNo(1)
  , m_History(NULL)
{
  wxLogTrace("SY_CALL", "+MDIFrame");

  const long flag =
    wxAUI_NB_DEFAULT_STYLE |
    wxAUI_NB_CLOSE_ON_ALL_TABS |
    wxAUI_NB_CLOSE_BUTTON |
    wxAUI_NB_WINDOWLIST_BUTTON |
    wxAUI_NB_SCROLL_BUTTONS;

  m_NotebookWithEditors = new exNotebook(
    this, this, (wxWindowID)NOTEBOOK_EDITORS, wxDefaultPosition, wxDefaultSize, flag);
  m_NotebookWithLists = new exNotebook(
    this, this, (wxWindowID)NOTEBOOK_LISTS, wxDefaultPosition, wxDefaultSize, flag);
  m_NotebookWithProjects = new exNotebook(
    this, this, (wxWindowID)NOTEBOOK_PROJECTS, wxDefaultPosition, wxDefaultSize, flag);
  m_History = new exListViewFile(this,
    exListViewFile::LIST_HISTORY,
    exListViewFile::LIST_MENU_DEFAULT | exListViewFile::LIST_MENU_RBS);
  m_DirCtrl = new wxGenericDirCtrl(this,
    wxID_ANY,
    wxFileName(GetRecentFile()).GetFullPath());
  exSTC* asciiTable = new exSTC(this);
  asciiTable->AddAsciiTable();
  asciiTable->SetReadOnly(true);
  exFindToolBar* findbar = new exFindToolBar(this, this);

  GetManager().AddPane(m_NotebookWithEditors,
    wxAuiPaneInfo().CenterPane().MaximizeButton(true).Name("FILES").Caption(_("Files")));
  GetManager().AddPane(m_NotebookWithProjects,
    wxAuiPaneInfo().Left().MaximizeButton(true).BestSize(250, 250).Name("PROJECTS").Caption(_("Projects")));
  GetManager().AddPane(m_NotebookWithLists,
    wxAuiPaneInfo().Bottom().MaximizeButton(true).MinSize(250, 100).Name("OUTPUT").Caption(_("Output")));
  GetManager().AddPane(m_DirCtrl,
    wxAuiPaneInfo().Left().BestSize(400, 250).Name("DIRCTRL").Caption(_("Explorer")));
  GetManager().AddPane(asciiTable,
    wxAuiPaneInfo().Left().BestSize(400, 250).Name("ASCIITABLE").Caption(_("Ascii Table")));
  GetManager().AddPane(m_History,
    wxAuiPaneInfo().Left().BestSize(400, 250).Name("HISTORY").Caption(_("History")));
  GetManager().AddPane(findbar,
    wxAuiPaneInfo().ToolbarPane().Bottom().Name("FINDBAR").Caption(_("Findbar")));

  const wxString perspective = exApp::GetConfig("Perspective");

  if (perspective.empty())
  {
    GetManager().GetPane("PROJECTS").Hide();
    GetManager().GetPane("DIRCTRL").Hide();
    GetManager().GetPane("ASCIITABLE").Hide();
    GetManager().GetPane("HISTORY").Hide();
  }
  else
  {
    GetManager().LoadPerspective(perspective);
  }

  // Regardless of the perspective initially hide the next panels.
  GetManager().GetPane("OUTPUT").Hide();

  if (open_recent)
  {
    if (!GetRecentFile().empty())
    {
      OpenFile(exFileName(GetRecentFile()));
    }
    else
    {
      NewFile();
    }

    if (GetManager().GetPane("PROJECTS").IsShown())
    {
      if (!GetRecentProject().empty())
      {
        OpenFile(
          exFileName(GetRecentProject()),
          0,
          wxEmptyString,
          exSTCWithFrame::STC_OPEN_IS_PROJECT);
      }
    }
  }
  else
  {
    GetManager().GetPane("PROJECTS").Hide();
  }

  // End with update, so all changes in the manager are handled.
  GetManager().Update();

  wxLogTrace("SY_CALL", "-MDIFrame");
}

exListViewFile* MDIFrame::Activate(int type, const exLexer* lexer)
{
  if (type == exListViewFile::LIST_PROJECT)
  {
    return GetCurrentProject();
  }
  else
  {
    exListViewFile* list = AddPage(type, lexer);
    GetManager().GetPane("OUTPUT").Show();
    GetManager().Update();
    return list;
  }
}

exListViewFile* MDIFrame::AddPage(int type, const exLexer* lexer)
{
  const wxString name = exListViewFile::GetTypeDescription((exListViewFile::ListType)type) +
    (lexer != NULL ?  " " + lexer->GetScintillaLexer(): wxString(wxEmptyString));

  exListViewFile* list = (exListViewFile*)m_NotebookWithLists->GetPageByKey(name);

  if (list == NULL && type != exListViewFile::LIST_PROJECT)
  {
    list = new exListViewFile(
      m_NotebookWithLists,
      (exListViewFile::ListType)type,
      exListViewFile::LIST_MENU_DEFAULT | exListViewFile::LIST_MENU_RBS,
      lexer);
    m_NotebookWithLists->AddPage(list, name, name, true);
  }

  return list;
}

bool MDIFrame::AllowCloseAll(wxWindowID id)
{
  switch (id)
  {
  case NOTEBOOK_EDITORS: return m_NotebookWithEditors->ForEach(ID_ALL_STC_CLOSE); break;
  case NOTEBOOK_PROJECTS: return exForEach(m_NotebookWithProjects, ID_LIST_ALL_CLOSE); break;
  }

  return true;
}


void MDIFrame::ConfigDialogApplied(wxWindowID dialogid)
{
  if (dialogid == ID_OPTION_EDITOR)
  {
    m_NotebookWithEditors->ForEach(ID_ALL_STC_CONFIG_GET);
  }
}

exListView* MDIFrame::GetListView()
{
  if (GetCurrentListView() != NULL)
  {
    return GetCurrentListView();
  }
  else
  {
    return GetCurrentProject();
  }
}

exListViewFile* MDIFrame::GetCurrentProject()
{
  if (!m_NotebookWithProjects->IsShown() || m_NotebookWithProjects->GetPageCount() == 0)
  {
    return NULL;
  }

  return (exListViewFile*)m_NotebookWithProjects->GetPage(m_NotebookWithProjects->GetSelection());
}

exSTCWithFrame* MDIFrame::GetCurrentSTC()
{
  if (!m_NotebookWithEditors->IsShown() || m_NotebookWithEditors->GetPageCount() == 0)
  {
    return NULL;
  }

  return (exSTCWithFrame*)m_NotebookWithEditors->GetPage(m_NotebookWithEditors->GetSelection());
}

void MDIFrame::NewFile(bool as_project)
{
  const wxString name = (as_project ? _("Project") : _("Textfile"));
  const wxString text = wxString::Format("%s%d", name.c_str(), m_NewFileNo++);

  exNotebook* notebook = (as_project ? m_NotebookWithProjects : m_NotebookWithEditors);
  wxWindow* page;

  if (as_project)
  {
    const wxFileName fn(
      wxStandardPaths::Get().GetUserDataDir(),
      text);

    page = new exListViewFile(notebook,
      fn.GetFullPath(),
      project_wildcard,
      exListViewFile::LIST_MENU_DEFAULT | exListViewFile::LIST_MENU_RBS);

    SetTitle(wxEmptyString, text);
  }
  else
  {
    page = new exSTCWithFrame(notebook);

    ((exSTCWithFrame*)page)->FileNew(text);
    ((exSTCWithFrame*)page)->PropertiesMessage();
  }

  notebook->AddPage(
    page,
    text,
    text,
    true
#ifdef USE_NOTEBOOK_IMAGE
    ,wxArtProvider::GetBitmap(wxART_NORMAL_FILE)
#endif
    );

  const wxString pane = (as_project ? "PROJECTS" : "FILES");
  GetManager().GetPane(pane).Show();
  GetManager().Update();
}

void MDIFrame::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    if (!AllowCloseAll(NOTEBOOK_PROJECTS) || !AllowCloseAll(NOTEBOOK_EDITORS))
    {
      event.Veto();
      return;
    }
  }

#if wxUSE_CHECKBOX
  exApp::SetConfigBool("HexMode", GetHexModeCheckBox()->GetValue());
#endif
  exApp::SetConfig("Perspective", GetManager().SavePerspective());

  event.Skip();
}

void MDIFrame::OnCommand(wxCommandEvent& event)
{
  if (event.GetId() == ID_ALL_STC_CLOSE ||
      event.GetId() == ID_ALL_STC_PRINT ||
      event.GetId() == ID_ALL_STC_SAVE)
  {
    if (event.GetId() == ID_ALL_STC_PRINT)
    {
      if (wxMessageBox(wxString(_("Print all files")) + "?",
        _("Confirm"), wxOK | wxCANCEL | wxICON_QUESTION) != wxOK)
        return;
    }

    m_NotebookWithEditors->ForEach(event.GetId());
    return;
  }

  exSTCWithFrame* editor = GetCurrentSTC();
  exListViewFile* project = GetCurrentProject();

  // edit commands
  // Do not change the wxID* in wxID_LOWEST and wdID_HIGHEST,
  // as wxID_ABOUT etc. is used here and not in the editor.
  // That causes appl to hang.
  if ((event.GetId() == wxID_UNDO ||
       event.GetId() == wxID_REDO ||
       event.GetId() == wxID_JUMP_TO) ||
      (event.GetId() >= wxID_CUT && event.GetId() <= wxID_PROPERTIES) ||
      (event.GetId() >= ID_EDIT_STC_LOWEST && event.GetId() <= ID_EDIT_STC_HIGHEST)||
      (event.GetId() >= ID_STC_LOWEST && event.GetId() <= ID_STC_HIGHEST))
  {
    if (editor != NULL)
    {
      wxPostEvent(editor, event);
    }
  }
  // tool commands
  else if (event.GetId() >= ID_TOOL_LOWEST && event.GetId() <= ID_TOOL_HIGHEST)
  {
    wxWindow* focus = wxWindow::FindFocus();
    if (focus != NULL)
    {
      wxPostEvent(focus, event);
    }
    else if (editor != NULL)
    {
      wxPostEvent(editor, event);
    }
  }
  // view commands
  else if (event.GetId() >= wxID_VIEW_DETAILS &&  event.GetId() <= wxID_VIEW_LIST)
  {
    exForEach(m_NotebookWithProjects, event.GetId());

    long view = 0;
    switch (event.GetId())
    {
    case wxID_VIEW_DETAILS: view = wxLC_REPORT; break;
    case wxID_VIEW_LIST: view = wxLC_LIST; break;
    case wxID_VIEW_SMALLICONS: view = wxLC_SMALL_ICON; break;
    default: wxFAIL;
    }

    m_History->SetSingleStyle(view);
  }
  // the rest
  else switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(APPL_VERSION);
    info.AddDeveloper(wxVERSION_STRING);
    info.AddDeveloper(wxEX_VERSION_STRING);
#ifdef USE_OTL
    info.AddDeveloper(exOTLVersion());
#endif
#ifdef EX_PORTABLE
    info.SetDescription(
_("This program offers a portable text or binary editor\n\
with automatic syncing. All its config files are read\n\
and saved in the same directory as where the executable is."));

#else
    info.SetDescription(_("This program offers a text or binary editor with automatic syncing."));
#endif
    info.SetCopyright("(c) 1998-2009, Anton van Wezenbeek. " + wxString(_("All rights reserved.")));
    info.SetWebSite("http://svn.xp-dev.com/svn/antonvw-wxprojects/trunk/syncedit/release.htm");
    wxAboutBox(info);
    }
    break;
  case wxID_CLOSE:
    if (editor != NULL)
      m_NotebookWithEditors->DeletePage(editor->GetFileName().GetFullPath());
    SetTitle(wxEmptyString, wxEmptyString);
    break;
  case wxID_EXIT: Close(true); break;
  case wxID_HELP_CONTENTS:
    {
    wxFileName fn(
#ifdef EX_PORTABLE
      wxStandardPaths::Get().GetExecutablePath(),
#else
      wxStandardPaths::Get().GetDataDir(),
#endif
      wxTheApp->GetAppName() + ".htm");
    wxLaunchDefaultBrowser(fn.GetFullPath());
    }
    break;
  case wxID_NEW: NewFile(); break;
  case wxID_PREVIEW:
    if (GetFocusedSTC() != NULL)
    {
      GetFocusedSTC()->PrintPreview();
    }
    else if (GetFocusedListView() != NULL)
    {
      GetFocusedListView()->PrintPreview();
    }
    break;
  case wxID_PRINT:
    if (GetFocusedSTC() != NULL)
    {
      GetFocusedSTC()->Print();
    }
    else if (GetFocusedListView() != NULL)
    {
      GetFocusedListView()->Print();
    }
    break;
  case wxID_PRINT_SETUP:
    exApp::GetPrinter()->PageSetup();
    break;

  case wxID_SAVE:
    if (editor != NULL && editor->FileSave())
    {
      editor->PropertiesMessage();

      if (editor->GetFileName() == exApp::GetLexers()->GetFileName())
      {
        exApp::GetLexers()->Read();
        m_NotebookWithEditors->ForEach(ID_ALL_STC_SET_LEXER);
        // As the lexer might have changed, update status bar field as well.
        editor->UpdateStatusBar("PaneLexer");
      }
    }
    break;
  case wxID_SAVEAS:
    if (editor != NULL)
    {
      const wxString old_key = editor->GetFileName().GetFullPath(); // before FileSaveAs
      if (editor->FileSaveAs())
      {
        m_NotebookWithEditors->SetPageText(
          old_key,
          editor->GetFileName().GetFullPath(),
          editor->GetFileName().GetFullName());
        editor->PropertiesMessage();
      }
    }
    break;

  case wxID_EXECUTE: exListViewFile::ProcessRun(); break;
  case wxID_STOP:
    if (exListViewFile::ProcessIsRunning())
    {
      exListViewFile::ProcessStop();
    }
    break;

  case ID_EDIT_HEX_MODE:
#if wxUSE_CHECKBOX
      if (editor != NULL &&
         // Reopen the current file, in the new mode, if different from current mode.
         (((editor->GetFlags() & exSTC::STC_OPEN_HEX) > 0) != GetHexModeCheckBox()->GetValue()))
      {
        long flags = 0;
        if (GetHexModeCheckBox()->GetValue()) flags |= exSTC::STC_OPEN_HEX;
        editor->Open(editor->GetFileName().GetFullPath(),
          0, wxEmptyString, flags);
      }
#endif
    break;

  case ID_OPEN_LEXERS: OpenFile(exApp::GetLexers()->GetFileName()); break;
  case ID_OPEN_LOGFILE: OpenFile(exLogfileName()); break;

  case ID_OPTION_SVN_AND_COMPARATOR:
    {
    std::vector<exConfigItem> v;
    v.push_back(exConfigItem()); // a spacer
    v.push_back(exConfigItem("SVN", CONFIG_CHECKBOX));
    v.push_back(exConfigItem(_("Comparator"), CONFIG_FILEPICKERCTRL));
    exConfigDialog(this, exApp::GetConfig(), v, _("Set SVN And Comparator")).ShowModal();
    }
    break;
  case ID_OPTION_EDITOR:
    exSTC::ConfigDialog(_("Editor Options"),
      exSTC::STC_CONFIG_MODELESS | exSTC::STC_CONFIG_WITH_APPLY,
      this,
      event.GetId());
    break;
  case ID_OPTION_LIST_FONT:
    {
      std::vector<exConfigItem> v;
      v.push_back(exConfigItem(_("List Font"), CONFIG_FONTPICKERCTRL));

      if (exConfigDialog(
        this,
        exApp::GetConfig(),
        v,
        _("Set List Font")).ShowModal() == wxID_OK)
      {
        wxFont font(
          exApp::GetConfig(wxString(_("List Font")) + "/Size", 10),
          wxFONTFAMILY_DEFAULT,
          wxFONTSTYLE_NORMAL,
          wxFONTWEIGHT_NORMAL,
          false,
          exApp::GetConfig(wxString(_("List Font")) + "/Name"));

        exForEach(m_NotebookWithProjects, ID_LIST_ALL_ITEMS, font);
        exForEach(m_NotebookWithLists, ID_LIST_ALL_ITEMS, font);
        m_History->SetFont(font);
        m_History->ItemsUpdate();
      }
    }
    break;

  case wxID_SORT_ASCENDING:
    exApp::SetConfig("List/SortMethod", SORT_ASCENDING); break;
  case wxID_SORT_DESCENDING:
    exApp::SetConfig("List/SortMethod", SORT_DESCENDING); break;
  case ID_OPTION_LIST_SORT_TOGGLE:
    exApp::SetConfig("List/SortMethod", SORT_TOGGLE); break;

  case ID_PROCESS_SELECT: exProcessWithListView::ConfigDialog(); break;

  case ID_PROJECT_CLOSE:
    if (project != NULL)
      m_NotebookWithProjects->DeletePage(project->GetFileName().GetFullPath());
    SetTitle(wxEmptyString, wxEmptyString);
    break;
  case ID_PROJECT_NEW: NewFile(true); break;
  case ID_PROJECT_OPENTEXT:
    if (project != NULL)
    {
      if (project->GetContentsChanged())
      {
        if (project->Continue())
        {
          project->FileSave();
          SetTitle(wxEmptyString, project->GetFileName().GetName());
        }
        else
        {
          return;
        }
      }

      OpenFile(project->GetFileName());
    }
    break;
  case ID_PROJECT_SAVEAS:
    if (project != NULL)
    {
      const wxString dir = wxGetCwd();
      wxSetWorkingDirectory(wxStandardPaths::Get().GetUserDataDir());

      const wxString old_key = m_NotebookWithProjects->GetKeyByPage(project);

      if (project->FileSaveAs())
      {
        m_NotebookWithProjects->SetPageText(
          old_key,
          project->GetFileName().GetFullPath(),
          project->GetFileName().GetName());
        SetTitle(wxEmptyString, project->GetFileName().GetName());
      }

      wxSetWorkingDirectory(dir);
    }
    break;

  case ID_SORT_SYNC: exApp::ToggleConfig("List/SortSync"); break;

  case ID_SPLIT:
  {
    exSTCWithFrame* stc = new exSTCWithFrame(*editor);

    m_NotebookWithEditors->AddPage(
      stc,
      // key should be unique
      wxString::Format("stc%d", stc->GetId()),
      stc->GetFileName().GetFullName(),
      true
#ifdef USE_NOTEBOOK_IMAGE
      ,wxTheFileIconsTable->GetSmallImageList()->GetBitmap(stc->GetFileName().GetIcon())
#endif
      );

    stc->SetDocPointer(editor->GetDocPointer());
  }
  break;

  case ID_SVN_COMMIT: exSVN(SVN_COMMIT).ExecuteAndShowOutput(); break;
  case ID_SVN_DIFF: exSVN(SVN_DIFF).ExecuteAndShowOutput(); break;
  case ID_SVN_INFO: exSVN(SVN_INFO).ExecuteAndShowOutput(); break;
  case ID_SVN_LOG: exSVN(SVN_LOG).ExecuteAndShowOutput(); break;
  case ID_SVN_STAT: exSVN(SVN_STAT).ExecuteAndShowOutput(); break;

  case ID_SYNC_MODE:
#if wxUSE_CHECKBOX
    exApp::SetConfigBool("AllowSync", GetSyncCheckBox()->GetValue());
#endif
    break;

  case ID_TREE_RUN_MAKE:
  {
    wxSetWorkingDirectory(wxFileName(m_DirCtrl->GetFilePath()).GetPath());

    exListViewFile::ProcessRun(
      exApp::GetConfig(_("Make")) + wxString(" ") +
      exApp::GetConfig("MakeSwitch", "-f") + wxString(" ") +
      m_DirCtrl->GetFilePath());
  }
  break;

  case ID_TREE_SVN_CAT:
  {
    exSVN svn(SVN_CAT, m_DirCtrl->GetFilePath());
    const int result = svn.Execute();

    if (result == 0)
    {
      OpenFile(exFileName(m_DirCtrl->GetFilePath()), svn.GetOutput());
    }
    else if (result > 0)
    {
      svn.ShowOutput();
    }
  }
  break;
  case ID_TREE_SVN_DIFF: exSVN(SVN_DIFF, m_DirCtrl->GetFilePath()).ExecuteAndShowOutput(); break;
  case ID_TREE_SVN_LOG: exSVN(SVN_LOG, m_DirCtrl->GetFilePath()).ExecuteAndShowOutput(); break;
  case ID_TREE_OPEN: OpenFile(exFileName(m_DirCtrl->GetFilePath())); break;

  case ID_VIEW_ASCII_TABLE: TogglePane("ASCIITABLE"); break;
  case ID_VIEW_DIRCTRL: TogglePane("DIRCTRL");   break;
  case ID_VIEW_FILES: TogglePane("FILES"); break;
  case ID_VIEW_FINDBAR: TogglePane("FINDBAR"); break;
  case ID_VIEW_HISTORY: TogglePane("HISTORY"); break;
  case ID_VIEW_OUTPUT: TogglePane("OUTPUT"); break;
  case ID_VIEW_PROJECTS: TogglePane("PROJECTS"); break;

  default: event.Skip();
  }
}

void MDIFrame::OnTree(wxTreeEvent& event)
{
  const wxString selection = m_DirCtrl->GetFilePath();
  if (selection.empty()) return;

  const exFileName filename(selection);

  if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK)
  {
    wxMenu menu;
    menu.Append(ID_TREE_OPEN, _("&Open"));

    if (exApp::GetConfigBool("SVN"))
    {
      menu.AppendSeparator();
      wxMenu* svnmenu = new wxMenu;
      svnmenu->Append(ID_TREE_SVN_DIFF, exEllipsed(_("&Diff")));
      svnmenu->Append(ID_TREE_SVN_LOG, exEllipsed(_("&Log")));
      svnmenu->Append(ID_TREE_SVN_CAT, exEllipsed(_("&Cat")));
      menu.AppendSubMenu(svnmenu, "&SVN");

      if (filename.GetLexer().GetScintillaLexer() == "makefile")
      {
        menu.AppendSeparator();
        menu.Append(ID_TREE_RUN_MAKE, exEllipsed(_("&Make")));
      }
    }

    PopupMenu(&menu);
  }
  else
  {
    OpenFile(filename);
  }
}

void MDIFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  exListViewFile* project = GetCurrentProject();

  if (event.GetId() >= wxID_VIEW_DETAILS && event.GetId() <= wxID_VIEW_LIST)
  {
    event.Enable(
      (project != NULL && project->IsShown()) ||
      m_History->IsShown());

    if (project != NULL && project->IsShown())
    {
      switch (project->GetWindowStyle() & wxLC_MASK_TYPE)
      {
        case wxLC_LIST: event.Check(event.GetId() == wxID_VIEW_LIST); break;
        case wxLC_REPORT: event.Check(event.GetId() == wxID_VIEW_DETAILS); break;
        case wxLC_SMALL_ICON: event.Check(event.GetId() == wxID_VIEW_SMALLICONS); break;
        default: wxFAIL;
      }
    }
    else if (m_History->IsShown())
    {
      switch (m_History->GetWindowStyle() & wxLC_MASK_TYPE)
      {
        case wxLC_LIST: event.Check(event.GetId() == wxID_VIEW_LIST); break;
        case wxLC_REPORT: event.Check(event.GetId() == wxID_VIEW_DETAILS); break;
        case wxLC_SMALL_ICON: event.Check(event.GetId() == wxID_VIEW_SMALLICONS); break;
        default: wxFAIL;
      }
    }
  }
  else switch (event.GetId())
    {
    case wxID_EXECUTE: event.Enable(exProcessWithListView::IsSelected()); break;
    case wxID_STOP: event.Enable(exListViewFile::ProcessIsRunning()); break;

    case ID_ALL_STC_CLOSE:
    case ID_ALL_STC_PRINT:
    case ID_ALL_STC_SAVE:
      event.Enable(m_NotebookWithEditors->GetPageCount() > 2);
    break;

    case wxID_SORT_ASCENDING:
    case wxID_SORT_DESCENDING:
      event.Check(
        event.GetId() - wxID_SORT_ASCENDING == exApp::GetConfig("List/SortMethod",
        SORT_TOGGLE) - SORT_ASCENDING);
      break;
    case ID_OPTION_LIST_SORT_TOGGLE:
      event.Check(
        event.GetId() - ID_OPTION_LIST_SORT_TOGGLE == exApp::GetConfig("List/SortMethod",
        SORT_TOGGLE) - SORT_TOGGLE);
    break;

    case ID_PROJECT_CLOSE:
    case ID_PROJECT_SAVEAS:
      event.Enable(project != NULL && project->IsShown());
    break;
    case ID_PROJECT_OPENTEXT:
      event.Enable(project != NULL && !project->GetFileName().GetFullPath().empty());
      break;
    case ID_PROJECT_SAVE:
      event.Enable(project != NULL && project->GetContentsChanged());
      break;

    case ID_RECENT_FILE_MENU:
      event.Enable(!GetRecentFile().empty());
      break;
    case ID_RECENT_PROJECT_MENU:
      event.Enable(!GetRecentProject().empty());
      break;

    case ID_SORT_SYNC:
      event.Check(exApp::GetConfigBool("List/SortSync"));
    break;

    case ID_VIEW_ASCII_TABLE:
      event.Check(GetManager().GetPane("ASCIITABLE").IsShown());
      break;
    case ID_VIEW_DIRCTRL:
      event.Check(GetManager().GetPane("DIRCTRL").IsShown());
      break;
    case ID_VIEW_FILES:
      event.Check(GetManager().GetPane("FILES").IsShown());
      break;
    case ID_VIEW_FINDBAR:
      event.Check(GetManager().GetPane("FINDBAR").IsShown());
      break;
    case ID_VIEW_HISTORY:
      event.Check(GetManager().GetPane("HISTORY").IsShown());
      break;
    case ID_VIEW_OUTPUT:
      event.Check(GetManager().GetPane("OUTPUT").IsShown());
      break;
    case ID_VIEW_PROJECTS:
      event.Check(GetManager().GetPane("PROJECTS").IsShown());
      break;

    case wxID_PREVIEW:
    case wxID_PRINT:
      event.Enable(
        (GetFocusedSTC() != NULL && GetFocusedSTC()->GetLength() > 0) ||
        (GetFocusedListView() != NULL && GetFocusedListView()->GetItemCount() > 0));
      break;

    // The rest here depends on a current editor window. If none,
    // the events are disabled.
    default:
    {
      exSTCWithFrame* editor = GetCurrentSTC();
      if (editor == NULL || !editor->IsShown())
      {
        event.Enable(false);
        return;
      }

      event.Enable(true);

      switch (event.GetId())
      {
      case wxID_CLOSE: // nothing extra, just prevent wxFAIL from happening
        break;

      case wxID_FIND:
      case wxID_JUMP_TO:
      case wxID_REPLACE:
      case wxID_SAVEAS:
      case ID_EDIT_FIND_NEXT:
      case ID_EDIT_FOLD_ALL:
      case ID_EDIT_UNFOLD_ALL:
        event.Enable(editor->GetLength() > 0);
        break;

      case ID_EDIT_MACRO_PLAYBACK:
        event.Enable(editor->MacroIsRecorded() && !editor->MacroIsRecording());
        break;
      case ID_EDIT_MACRO_START_RECORD:
        event.Enable(editor->GetLength() > 0 && !editor->MacroIsRecording());
        break;
      case ID_EDIT_MACRO_STOP_RECORD:
        event.Enable(editor->MacroIsRecording());
        break;

      case ID_EDIT_TOGGLE_FOLD:
        event.Enable(
          editor->GetTextLength() > 0 &&
          editor->GetFoldLevel(editor->GetCurrentLine()) > wxSTC_FOLDLEVELBASE);
        break;

      case wxID_COPY:
        if (GetFocusedListView() != NULL)
        {
          event.Enable(GetFocusedListView()->GetSelectedItemCount() > 0);
        }
        else
        {
          event.Enable(!editor->GetSelectedText().empty());
        }
        break;

      case wxID_CUT:
        if (GetFocusedListView() != NULL)
        {
          event.Enable(GetFocusedListView()->GetSelectedItemCount() > 0);
        }
        else
        {
          event.Enable(
            !editor->GetReadOnly() &&
            !editor->GetSelectedText().empty());
        }
        break;

      case wxID_PASTE:
        if (GetFocusedListView() != NULL)
        {
          event.Enable(GetFocusedListView()->GetType() == exListViewFile::LIST_PROJECT);
        }
        else
        {
          event.Enable(editor->CanPaste());
        }
        break;

      case wxID_SAVE:
        event.Enable(
          !editor->GetFileName().GetFullPath().empty() &&
           editor->GetModify());
        break;
      case wxID_REDO:
        event.Enable(editor->CanRedo());
        break;
      case wxID_UNDO:
        event.Enable(editor->CanUndo());
        break;

      case ID_EDIT_CONTROL_CHAR:
        if (editor->GetReadOnly() && editor->GetSelectedText().length() != 1)
        {
          event.Enable(false);
        }
        break;

      default: event.Skip();
      }
    }
  }
}

bool MDIFrame::OpenFile(
  const exFileName& filename,
  const wxString& contents,
  long flags)
{
  const wxString key = filename.GetFullPath()+ exApp::GetConfig(_("Flags"));

  wxWindow* page = m_NotebookWithEditors->GetPageByKey(key, true);

  if (page == NULL)
  {
    exSTCWithFrame* editor = new exSTCWithFrame(m_NotebookWithEditors, exSTC::STC_MENU_DEFAULT, contents);
    editor->SetLexer(filename.GetLexer().GetScintillaLexer());

    m_NotebookWithEditors->AddPage(
      editor,
      key,
      filename.GetFullName() + " " + exApp::GetConfig(_("Flags")),
      true
#ifdef USE_NOTEBOOK_IMAGE
      ,wxTheFileIconsTable->GetSmallImageList()->GetBitmap(filename.GetIcon())
#endif
      );
  }

  return true;
}

bool MDIFrame::OpenFile(
  const exFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  wxLogTrace("SY_CALL", "+OpenFile");

  exNotebook* notebook = (flags & exSTCWithFrame::STC_OPEN_IS_PROJECT
    ? m_NotebookWithProjects : m_NotebookWithEditors);

  wxWindow* page = notebook->GetPageByKey(filename.GetFullPath(), true);

  if (flags & exSTCWithFrame::STC_OPEN_IS_PROJECT)
  {
    if (page == NULL)
    {
      exListViewFile* project = new exListViewFile(m_NotebookWithProjects,
        filename.GetFullPath(),
        project_wildcard,
        exListViewFile::LIST_MENU_DEFAULT | exListViewFile::LIST_MENU_RBS);

      m_NotebookWithProjects->AddPage(
        project,
        filename.GetFullPath(),
        filename.GetName(),
        true
#ifdef USE_NOTEBOOK_IMAGE
        ,wxArtProvider::GetBitmap(wxART_NORMAL_FILE)
#endif
        );
    }

    if (!GetManager().GetPane("PROJECTS").IsShown())
    {
      GetManager().GetPane("PROJECTS").Show();
      GetManager().Update();
    }

    SetTitle(wxEmptyString, filename.GetName());
  }
  else
  {
    if (!GetManager().GetPane("FILES").IsShown())
    {
      GetManager().GetPane("FILES").Show();

      if (GetManager().GetPane("PROJECTS").IsMaximized())
      {
        GetManager().GetPane("PROJECTS").Restore();
      }

      GetManager().Update();
    }

    exSTCWithFrame* editor = (exSTCWithFrame*)page;

    if (page == NULL)
    {
#if wxUSE_CHECKBOX
      if (GetHexModeCheckBox()->GetValue())
        flags |= exSTC::STC_OPEN_HEX;
#endif

      wxLogTrace("SY_CALL", "+exSTCWithFrame");

      editor = new exSTCWithFrame(m_NotebookWithEditors,
        filename.GetFullPath(),
        line_number,
        match,
        flags);

      wxLogTrace("SY_CALL", "-exSTCWithFrame");

      m_NotebookWithEditors->AddPage(
        editor,
        filename.GetFullPath(),
        filename.GetFullName(),
        true
#ifdef USE_NOTEBOOK_IMAGE
        ,wxTheFileIconsTable->GetSmallImageList()->GetBitmap(filename.GetIcon())
#endif
        );

      if (GetManager().GetPane("DIRCTRL").IsShown())
      {
        m_DirCtrl->SetPath(filename.GetFullPath());
      }
    }
    else if (line_number > 0)
    {
      editor->GotoLineAndSelect(line_number, match);
    }
  }

  wxLogTrace("SY_CALL", "-OpenFile");

  return true;
}

void MDIFrame::SyncCloseAll(wxWindowID id)
{
  switch (id)
  {
  case NOTEBOOK_EDITORS:
    NewFile();
    break;
  case NOTEBOOK_LISTS:
    GetManager().GetPane("OUTPUT").Hide();
    GetManager().Update();
    break;
  case NOTEBOOK_PROJECTS:
    GetManager().GetPane("PROJECTS").Hide();
    GetManager().Update();
    break;
  default: wxFAIL;
  }
}
