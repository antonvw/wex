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
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/stdpaths.h> // for wxStandardPaths
#include <wx/extension/configdlg.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/otl.h>
#include <wx/extension/printing.h>
#include <wx/extension/svn.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/process.h>
#include <wx/extension/report/stc.h>
#include <wx/extension/report/util.h>
#include "frame.h"
#include "defs.h"
#include "version.h"

BEGIN_EVENT_TABLE(MDIFrame, Frame)
#if wxUSE_CHECKBOX
  EVT_CHECKBOX(ID_EDIT_HEX_MODE, MDIFrame::OnCommand)
  EVT_CHECKBOX(ID_SYNC_MODE, MDIFrame::OnCommand)
#endif
  EVT_CLOSE(MDIFrame::OnClose)
  EVT_MENU(wxID_DELETE, MDIFrame::OnCommand)
  EVT_MENU(wxID_EXECUTE, MDIFrame::OnCommand)
  EVT_MENU(wxID_JUMP_TO, MDIFrame::OnCommand)
  EVT_MENU(wxID_SELECTALL, MDIFrame::OnCommand)
  EVT_MENU(wxID_STOP, MDIFrame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, MDIFrame::OnCommand)
  EVT_MENU_RANGE(wxID_CLOSE, wxID_PREFERENCES, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_APPL_LOWEST, ID_APPL_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_ALL_LOWEST, ID_ALL_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_STC_LOWEST, ID_STC_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_SVN_LOWEST, ID_SVN_HIGHEST, MDIFrame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, MDIFrame::OnCommand)
  EVT_TREE_ITEM_ACTIVATED(wxID_TREECTRL, MDIFrame::OnTree)
  EVT_TREE_ITEM_RIGHT_CLICK(wxID_TREECTRL, MDIFrame::OnTree)
  EVT_UPDATE_UI(ID_ALL_STC_CLOSE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_ALL_STC_SAVE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_COPY, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_CUT, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_EXECUTE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_JUMP_TO, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PRINT, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PREVIEW, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PASTE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UNDO, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REDO, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_STOP, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_CONTROL_CHAR, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_PLAYBACK, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_START_RECORD, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_STOP_RECORD, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_PROJECT_SAVE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_FILE_MENU, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_PROJECT_MENU, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SORT_SYNC, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(wxID_SAVE, wxID_SAVEAS, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_EDIT_FIND_NEXT, ID_EDIT_FIND_PREVIOUS, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_EDIT_TOGGLE_FOLD, ID_EDIT_UNFOLD_ALL, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_OPTION_LIST_SORT_ASCENDING, ID_OPTION_LIST_SORT_TOGGLE, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_PROJECT_OPENTEXT, ID_PROJECT_SAVEAS, MDIFrame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_VIEW_PANE_FIRST + 1, ID_VIEW_PANE_LAST - 1, MDIFrame::OnUpdateUI)
END_EVENT_TABLE()

MDIFrame::MDIFrame(bool open_recent)
  : Frame()
  , m_NewFileNo(1)
  , m_NewProjectNo(1)
  , m_History(NULL)
  , m_ProjectWildcard(_("Project Files") + " (*.prj)|*.prj")
{
  wxLogTrace("SY_CALL", "+MDIFrame");

  const long flag =
    wxAUI_NB_DEFAULT_STYLE |
    wxAUI_NB_CLOSE_ON_ALL_TABS |
    wxAUI_NB_CLOSE_BUTTON |
    wxAUI_NB_WINDOWLIST_BUTTON |
    wxAUI_NB_SCROLL_BUTTONS;

  m_NotebookWithEditors = new wxExNotebook(
    this, this, (wxWindowID)NOTEBOOK_EDITORS, wxDefaultPosition, wxDefaultSize, flag);
  m_NotebookWithLists = new wxExNotebook(
    this, this, (wxWindowID)NOTEBOOK_LISTS, wxDefaultPosition, wxDefaultSize, flag);
  m_NotebookWithProjects = new wxExNotebook(
    this, this, (wxWindowID)NOTEBOOK_PROJECTS, wxDefaultPosition, wxDefaultSize, flag);
  m_History = new wxExListViewWithFrame(this, this,
    wxExListViewStandard::LIST_HISTORY,
    wxExListViewStandard::LIST_MENU_DEFAULT);
  m_DirCtrl = new wxGenericDirCtrl(this,
    wxID_ANY,
    wxFileName(GetRecentFile()).GetFullPath());
  wxExSTC* asciiTable = new wxExSTC(this);
  asciiTable->AddAsciiTable();
  asciiTable->SetReadOnly(true);
  wxExFindToolBar* findbar = new wxExFindToolBar(this, this);

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

  const wxString perspective = wxConfigBase::Get()->Read("Perspective");

  if (perspective.empty())
  {
    GetManager().GetPane("PROJECTS").Hide();
    GetManager().GetPane("DIRCTRL").Hide();
    GetManager().GetPane("ASCIITABLE").Hide();
    GetManager().GetPane("HISTORY").Hide();
    GetManager().GetPane("FINDBAR").Hide();
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
      OpenFile(wxExFileName(GetRecentFile()));
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
          wxExFileName(GetRecentProject()),
          0,
          wxEmptyString,
          wxExSTCWithFrame::STC_OPEN_IS_PROJECT);
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

wxExListViewStandard* MDIFrame::Activate(
  wxExListViewStandard::ListType type, 
  const wxExLexer* lexer)
{
  if (type == wxExListViewStandard::LIST_FILE)
  {
    return GetProject();
  }
  else
  {
    wxExListViewWithFrame* list = AddPage(type, lexer);
    GetManager().GetPane("OUTPUT").Show();
    GetManager().Update();
    return list;
  }
}

wxExListViewWithFrame* MDIFrame::AddPage(
  wxExListViewStandard::ListType type, 
  const wxExLexer* lexer)
{
  const wxString name = wxExListViewStandard::GetTypeDescription(type) +
    (lexer != NULL ?  " " + lexer->GetScintillaLexer(): wxString(wxEmptyString));

  wxExListViewWithFrame* list = 
    (wxExListViewWithFrame*)m_NotebookWithLists->GetPageByKey(name);

  if (list == NULL && type != wxExListViewStandard::LIST_FILE)
  {
    list = new wxExListViewWithFrame(
      m_NotebookWithLists, this,
      (wxExListViewStandard::ListType)type,
      wxID_ANY,
      wxExListViewStandard::LIST_MENU_DEFAULT,
      lexer);
    m_NotebookWithLists->AddPage(list, name, name, true);
  }

  return list;
}

bool MDIFrame::AllowCloseAll(wxWindowID id)
{
  switch (id)
  {
  case NOTEBOOK_EDITORS: 
    return m_NotebookWithEditors->ForEach(ID_ALL_STC_CLOSE); 
    break;
  case NOTEBOOK_PROJECTS: 
    return wxExForEach(m_NotebookWithProjects, ID_LIST_ALL_CLOSE); 
    break;
  default:
    wxFAIL;
    break;
  }

  return true;
}

void MDIFrame::OnCommandConfigDialog(
  wxWindowID dialogid,
  int commandid)
{
  if (dialogid == ID_OPTION_EDITOR)
  {
    if (commandid != wxID_CANCEL)
    {
      m_NotebookWithEditors->ForEach(ID_ALL_STC_CONFIG_GET);
    }
  }
  else
  {
    Frame::OnCommandConfigDialog(dialogid, commandid);
  }
}

bool MDIFrame::DialogProjectOpen()
{
  wxFileDialog dlg(this,
    _("Select Projects"),
    wxStandardPaths::Get().GetUserDataDir(),
    wxEmptyString,
    m_ProjectWildcard,
    wxFD_OPEN | wxFD_MULTIPLE);

  if (dlg.ShowModal() == wxID_CANCEL) return false;

  wxArrayString files;
  dlg.GetPaths(files);
  wxExOpenFiles(this, files, wxExSTCWithFrame::STC_OPEN_IS_PROJECT);

  return true;
}

wxExListView* MDIFrame::GetListView()
{
  if (m_History->HasFocus())
  {
    return m_History;
  }
  else if (
    !m_NotebookWithLists->IsShown() || 
     m_NotebookWithLists->GetPageCount() == 0)
  {
    return NULL;
  }
  else
  {
    return (wxExListView*)m_NotebookWithLists->GetPage(
      m_NotebookWithLists->GetSelection());
  }
}

wxExListViewFile* MDIFrame::GetProject()
{
  if (
    !m_NotebookWithProjects->IsShown() || 
     m_NotebookWithProjects->GetPageCount() == 0)
  {
    return NULL;
  }
  else
  {
    return (wxExListViewFile*)m_NotebookWithProjects->
      GetPage(m_NotebookWithProjects->GetSelection());
  }
}

wxExSTC* MDIFrame::GetSTC()
{
  if (
    !m_NotebookWithEditors->IsShown() || 
     m_NotebookWithEditors->GetPageCount() == 0)
  {
    return NULL;
  }
  else
  {
    return (wxExSTC*)m_NotebookWithEditors->GetPage(
      m_NotebookWithEditors->GetSelection());
  }
}

void MDIFrame::NewFile(bool as_project)
{
  const wxString name = (as_project ? _("project") : _("textfile"));
  const int use_no = (as_project ? m_NewProjectNo : m_NewFileNo);
  const wxString text = wxString::Format("%s%d", name.c_str(), use_no);
  wxString key;

  wxExNotebook* notebook = (as_project ? 
    m_NotebookWithProjects : 
    m_NotebookWithEditors);

  wxWindow* page;

  if (as_project)
  {
    const wxFileName fn(
      wxStandardPaths::Get().GetUserDataDir(),
      text + ".prj");

    key = fn.GetFullPath();

    page = new wxExListViewFile(notebook,
      this,
      key,
      wxID_ANY,
      wxExListViewStandard::LIST_MENU_DEFAULT);

    ((wxExListViewFile*)page)->FileNew(fn);
    SetTitle(wxEmptyString, text);

    m_NewProjectNo++;
  }
  else
  {
#if wxUSE_CHECKBOX
    if (GetHexModeCheckBox()->GetValue())
    {
      // In hex mode we cannot edit the file.
      return;
    }
#endif

    key = text;
    page = new wxExSTCWithFrame(notebook, this);

    ((wxExSTCWithFrame*)page)->FileNew(text);

    m_NewFileNo++;
  }

  notebook->AddPage(
    page,
    key,
    text,
    true,
    wxArtProvider::GetBitmap(wxART_NORMAL_FILE));

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
  wxConfigBase::Get()->Write("HexMode", GetHexModeCheckBox()->GetValue());
#endif
  wxConfigBase::Get()->Write("Perspective", GetManager().SavePerspective());

  event.Skip();
}

void MDIFrame::OnCommand(wxCommandEvent& event)
{
  if (event.GetId() == ID_ALL_STC_CLOSE ||
      event.GetId() == ID_ALL_STC_SAVE)
  {
    m_NotebookWithEditors->ForEach(event.GetId());
    return;
  }

  if (event.GetId() > ID_EDIT_SVN_LOWEST && 
      event.GetId() < ID_EDIT_SVN_HIGHEST)
  {
    wxExSVN svn(event.GetId(), m_DirCtrl->GetFilePath());

    if (event.GetId() == ID_EDIT_SVN_CAT ||
        event.GetId() == ID_EDIT_SVN_BLAME)
    {
      if (svn.Execute(this) == wxID_OK)
      {
        OpenFile(
          wxExFileName(m_DirCtrl->GetFilePath()), 
          svn.GetCommandWithFlags(),
          svn.GetOutput());
      }
      else
      {
        svn.ShowOutput(this);
      }
    }
    else
    {
      svn.ExecuteAndShowOutput(this);
    }

    return;
  }

  wxExSTC* editor = GetSTC();
  wxExListViewFile* project = GetProject();

  if (event.GetId() == ID_EDIT_NEXT)
  {
    m_NotebookWithEditors->AdvanceSelection();
    return;
  }
  else if (event.GetId() == ID_EDIT_PREVIOUS)
  {
    m_NotebookWithEditors->AdvanceSelection(false);
    return;
  }

  // edit commands
  // Do not change the wxID* in wxID_LOWEST and wdID_HIGHEST,
  // as wxID_ABOUT etc. is used here and not in the editor.
  // That causes appl to hang.
  if ((event.GetId() == wxID_UNDO ||
       event.GetId() == wxID_REDO ||
       event.GetId() == wxID_DELETE ||
       event.GetId() == wxID_SELECTALL ||
       event.GetId() == wxID_JUMP_TO) ||
      (event.GetId() >= wxID_CUT && event.GetId() <= wxID_CLEAR) ||
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

#ifdef wxExUSE_OTL
    info.AddDeveloper(wxExOTL::Version());
#endif

    wxString description(
      _("This program offers a portable text or binary editor\n"
        "with automatic syncing."));

#ifdef wxExUSE_PORTABLE
    description +=
      _(" All its config files are read\n"
        "and saved in the same directory as where the executable is.");
#endif

    info.SetDescription(description);
    info.SetCopyright("(c) 1998-2010, Anton van Wezenbeek. " + wxString(_("All rights reserved.")));
    info.SetWebSite("http://syncped.1.xpdev-hosted.com/index.htm");
    wxAboutBox(info);
    }
    break;
  case wxID_CLOSE:
    if (editor != NULL)
    {
      m_NotebookWithEditors->DeletePage(editor->GetFileName().GetFullPath());
      SetTitle(wxEmptyString, wxEmptyString);
    }
    break;
  case wxID_EXIT: Close(true); break;
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
    wxExPrinting::Get()->GetHtmlPrinter()->PageSetup();
    break;

  case wxID_SAVE:
    if (editor != NULL)
    {
      editor->FileSave();

      // Compare fullpath, otherwise the Read still reads in the old one.
      if (editor->GetFileName() == 
          wxExLexers::Get()->GetFileName())
      {
        wxExLexers::Get()->Read();
        m_NotebookWithEditors->ForEach(ID_ALL_STC_SET_LEXER);

        // As the lexer might have changed, update status bar field as well.
#if wxUSE_STATUSBAR
        editor->UpdateStatusBar("PaneLexer");
#endif
      }
    }
    break;
  case wxID_SAVEAS:
    if (editor != NULL)
    {
      const wxString old_key = editor->GetFileName().GetFullPath();

      if (!event.GetString().empty())
      {
        editor->FileSave(event.GetString());
      }
      else
      {
        wxExFileDialog dlg(
          this, 
          editor, 
          _("File Save As"), 
          wxFileSelectorDefaultWildcardStr, 
          wxFD_SAVE);

        if (dlg.ShowModal() != wxID_OK)
        {
          return;
        }

        editor->FileSave(dlg.GetPath());
      }

      m_NotebookWithEditors->SetPageText(
        old_key,
        editor->GetFileName().GetFullPath(),
        editor->GetFileName().GetFullName());
    }
    editor->PropertiesMessage();
    break;

  case wxID_EXECUTE: ProcessRun(); break;
  case wxID_STOP: ProcessStop(); break;

  case ID_EDIT_HEX_MODE:
#if wxUSE_CHECKBOX
      if (editor != NULL &&
         // Reopen the current file, in the new mode, if different from current mode.
         (((editor->GetFlags() & wxExSTC::STC_OPEN_HEX) > 0) != GetHexModeCheckBox()->GetValue()))
      {
        long flags = 0;
        if (GetHexModeCheckBox()->GetValue()) flags |= wxExSTC::STC_OPEN_HEX;
        wxExFileDialog dlg(this, editor);
        if (dlg.ShowModalIfChanged() == wxID_CANCEL) return;
        editor->Open(editor->GetFileName().GetFullPath(),
          0, wxEmptyString, flags);
      }
#endif
    break;

  case ID_OPEN_LEXERS: OpenFile(wxExLexers::Get()->GetFileName()); break;
  case ID_OPEN_LOGFILE: OpenFile(wxExLog::Get()->GetFileName()); break;

  case ID_OPTION_SVN_AND_COMPARATOR: wxExSVN::Get()->ConfigDialog(this);
    break;

  case ID_OPTION_EDITOR:
    wxExSTC::ConfigDialog(this,
      _("Editor Options"),
      wxExSTC::STC_CONFIG_MODELESS | wxExSTC::STC_CONFIG_WITH_APPLY,
      event.GetId());
    break;
  case ID_OPTION_LIST_FONT:
    {
      std::vector<wxExConfigItem> v;
      v.push_back(wxExConfigItem(_("List Font"), CONFIG_FONTPICKERCTRL));

      if (wxExConfigDialog(
        this,
        v,
        _("Set List Font")).ShowModal() == wxID_OK)
      {
        const wxFont font(
          wxConfigBase::Get()->ReadObject(_("List Font"), 
            wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));

        wxExForEach(m_NotebookWithProjects, ID_LIST_ALL_ITEMS, font);
        wxExForEach(m_NotebookWithLists, ID_LIST_ALL_ITEMS, font);
        m_History->SetFont(font);
        m_History->ItemsUpdate();
      }
    }
    break;

  case ID_OPTION_LIST_READONLY_COLOUR:
    {
      if (!wxConfigBase::Get()->Exists(_("List Colour")))
      {
        wxConfigBase::Get()->Write(_("List Colour"), wxColour("RED"));
      }

      std::vector<wxExConfigItem> v;
      v.push_back(wxExConfigItem(_("List Colour"), CONFIG_COLOUR));

      // text also used in menu
      wxExConfigDialog(
        this,
        v,
        _("Set List Read Only Colour")).ShowModal();
    }
    break;

  case ID_OPTION_LIST_SORT_ASCENDING:
    wxConfigBase::Get()->Write("List/SortMethod", (long)SORT_ASCENDING); break;
  case ID_OPTION_LIST_SORT_DESCENDING:
    wxConfigBase::Get()->Write("List/SortMethod", (long)SORT_DESCENDING); break;
  case ID_OPTION_LIST_SORT_TOGGLE:
    wxConfigBase::Get()->Write("List/SortMethod", (long)SORT_TOGGLE); break;

  case ID_PROCESS_SELECT: wxExProcess::Get()->ConfigDialog(this); break;

  case ID_PROJECT_CLOSE:
    if (project != NULL)
    {
      m_NotebookWithProjects->DeletePage(project->GetFileName().GetFullPath());
      SetTitle(wxEmptyString, wxEmptyString);
    }
    break;
  case ID_PROJECT_NEW: NewFile(true); break;
    case ID_PROJECT_OPEN:
      DialogProjectOpen();
      break;
  case ID_PROJECT_OPENTEXT:
    if (project != NULL)
    {
      wxExFileDialog dlg(this, project);
      if (dlg.ShowModalIfChanged() != wxID_CANCEL)
      {
        project->FileSave();
        SetTitle(wxEmptyString, project->GetFileName().GetName());
        OpenFile(project->GetFileName());
      }
    }
    break;
  case ID_PROJECT_SAVEAS:
    if (project != NULL)
    {
      const wxString old_key = m_NotebookWithProjects->GetKeyByPage(project);

      wxExFileDialog dlg(
        this, project, 
        _("Project Save As"), 
        m_ProjectWildcard, 
        wxFD_SAVE);

      if (dlg.ShowModal() == wxID_OK)
      {
        project->FileSave(dlg.GetPath());

        m_NotebookWithProjects->SetPageText(
          old_key,
          project->GetFileName().GetFullPath(),
          project->GetFileName().GetName());
        SetTitle(wxEmptyString, project->GetFileName().GetName());
      }
    }
    break;

  case ID_SORT_SYNC: 
    wxConfigBase::Get()->Write("List/SortSync", 
      !wxConfigBase::Get()->ReadBool("List/SortSync", true)); 
    break;

  case ID_SPLIT:
  {
    wxExSTCWithFrame* stc = new wxExSTCWithFrame(*editor, this);

    m_NotebookWithEditors->AddPage(
      stc,
      // key should be unique
      wxString::Format("stc%d", stc->GetId()),
      stc->GetFileName().GetFullName(),
      true,
      wxTheFileIconsTable->GetSmallImageList()->GetBitmap(stc->GetFileName().GetIconID()));

    stc->SetDocPointer(editor->GetDocPointer());
  }
  break;

  case ID_SVN_ADD: wxExSVN(wxExSVN::SVN_ADD).ExecuteAndShowOutput(this); break;
  case ID_SVN_COMMIT: wxExSVN(wxExSVN::SVN_COMMIT).ExecuteAndShowOutput(this); break;
  case ID_SVN_DIFF: wxExSVN(wxExSVN::SVN_DIFF).ExecuteAndShowOutput(this); break;
  case ID_SVN_HELP: wxExSVN(wxExSVN::SVN_HELP).ExecuteAndShowOutput(this); break;
  case ID_SVN_INFO: wxExSVN(wxExSVN::SVN_INFO).ExecuteAndShowOutput(this); break;
  case ID_SVN_LOG: wxExSVN(wxExSVN::SVN_LOG).ExecuteAndShowOutput(this); break;
  case ID_SVN_LS: wxExSVN(wxExSVN::SVN_LS).ExecuteAndShowOutput(this); break;
  case ID_SVN_STAT: wxExSVN(wxExSVN::SVN_STAT).ExecuteAndShowOutput(this); break;
  case ID_SVN_UPDATE: wxExSVN(wxExSVN::SVN_UPDATE).ExecuteAndShowOutput(this); break;

  case ID_SYNC_MODE:
#if wxUSE_CHECKBOX
    wxConfigBase::Get()->Write("AllowSync", GetSyncCheckBox()->GetValue());
#endif
    break;

  case ID_TREE_OPEN: OpenFile(wxExFileName(m_DirCtrl->GetFilePath())); break;
  case ID_TREE_RUN_MAKE: wxExMake(this, wxFileName(m_DirCtrl->GetFilePath())); break;

  case ID_VIEW_ASCII_TABLE: TogglePane("ASCIITABLE"); break;
  case ID_VIEW_DIRCTRL: TogglePane("DIRCTRL"); break;
  case ID_VIEW_FILES: TogglePane("FILES"); 
    if (!GetManager().GetPane("FILES").IsShown())
    {
      if (GetManager().GetPane("PROJECTS").IsShown())
      {
        GetManager().GetPane("PROJECTS").Maximize();
      }
    }
    break;
  case ID_VIEW_FINDBAR: TogglePane("FINDBAR"); break;
  case ID_VIEW_HISTORY: 
    TogglePane("HISTORY");
#if wxUSE_STATUSBAR
    if (GetManager().GetPane("HISTORY").IsShown())
    {
      m_History->UpdateStatusBar();
    }
    else
    {
      StatusText(wxEmptyString, "PaneItems");
    }
#endif
    break;
  case ID_VIEW_OUTPUT: TogglePane("OUTPUT"); break;
  case ID_VIEW_PROJECTS: TogglePane("PROJECTS"); break;

  default: 
    wxFAIL;
    break;
  }
}

void MDIFrame::OnTree(wxTreeEvent& event)
{
  const wxString selection = m_DirCtrl->GetFilePath();

  if (selection.empty()) 
  {
    event.Skip();
    return;
  }

  const wxExFileName filename(selection);

  if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK)
  {
    wxExMenu menu;
    menu.Append(ID_TREE_OPEN, _("&Open"));

    if (wxExSVN::Get()->DirExists(filename))
    {
      menu.AppendSeparator();
      menu.AppendSVN();
    }

    if (filename.GetLexer().GetScintillaLexer() == "makefile")
    {
      menu.AppendSeparator();
      menu.Append(ID_TREE_RUN_MAKE, "&Make");
    }

    PopupMenu(&menu);
  }
  else if (event.GetEventType() == wxEVT_COMMAND_TREE_ITEM_ACTIVATED)
  {
    OpenFile(filename);
  }
  else
  {
    wxFAIL;
  }
}

void MDIFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  wxExListView* list = GetFocusedListView();

  switch (event.GetId())
  {
    case wxID_EXECUTE: 
      event.Enable( wxExProcess::Get()->IsSelected() &&
                   !wxExProcess::Get()->IsRunning()); 
      break;
    case wxID_STOP: event.Enable(wxExProcess::Get()->IsRunning()); break;

    case ID_ALL_STC_CLOSE:
    case ID_ALL_STC_SAVE:
      event.Enable(m_NotebookWithEditors->GetPageCount() > 2);
    break;

    case ID_OPTION_LIST_SORT_ASCENDING:
    case ID_OPTION_LIST_SORT_DESCENDING:
    case ID_OPTION_LIST_SORT_TOGGLE:
      event.Check(
        event.GetId() - ID_OPTION_LIST_SORT_ASCENDING == 
        wxConfigBase::Get()->ReadLong("List/SortMethod", SORT_TOGGLE) - SORT_ASCENDING);
    break;

    case ID_PROJECT_CLOSE:
    case ID_PROJECT_SAVEAS:
      event.Enable(GetProject() != NULL && GetProject()->IsShown());
    break;
    case ID_PROJECT_OPENTEXT:
      event.Enable(GetProject() != NULL && !GetProject()->GetFileName().GetFullPath().empty());
      break;
    case ID_PROJECT_SAVE:
      event.Enable(GetProject() != NULL && GetProject()->GetContentsChanged());
      break;

    case ID_RECENT_FILE_MENU:
      event.Enable(!GetRecentFile().empty());
      break;
    case ID_RECENT_PROJECT_MENU:
      event.Enable(!GetRecentProject().empty());
      break;

    case ID_SORT_SYNC:
      event.Check(wxConfigBase::Get()->ReadBool("List/SortSync", true));
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

    default:
    {
      wxExSTC* editor = GetSTC();
      wxExListViewFile* list = (wxExListViewFile*)GetFocusedListView();

      if (list == NULL && editor != NULL && editor->IsShown())
      {
        event.Enable(true);

        switch (event.GetId())
        {
        case wxID_JUMP_TO:
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
          event.Enable(!editor->GetSelectedText().empty());
          break;

        case wxID_CUT:
          event.Enable(
            !editor->GetReadOnly() &&
            !editor->GetSelectedText().empty());
          break;

        case wxID_PASTE:
          event.Enable(editor->CanPaste());
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

        default:
          wxFAIL;
        }
      }
      else if (list != NULL)
      {
        event.Enable(false);

        if (list != NULL)
        {
          switch (event.GetId())
          {
          case wxID_COPY:
          case wxID_CUT:
            event.Enable(list->GetSelectedItemCount() > 0);
            break;

          case wxID_PASTE:
            event.Enable(list->GetType() == wxExListViewStandard::LIST_FILE);
            break;

          default:
            // No wxFAIL;, too many events here.
            ;
          }
        }
      }
    }
  }
}

bool MDIFrame::OpenFile(
  const wxExFileName& filename,
  const wxString& unique,
  const wxString& contents)
{
  const wxString key = filename.GetFullPath() + unique;

  wxWindow* page = m_NotebookWithEditors->GetPageByKey(key);

  if (page == NULL)
  {
    wxExSTCWithFrame* editor = new wxExSTCWithFrame(
      m_NotebookWithEditors, 
      this,
      contents);

    editor->SetLexer(filename.GetLexer().GetScintillaLexer());

    m_NotebookWithEditors->AddPage(
      editor,
      key,
      filename.GetFullName() + " " + unique,
      true,
      wxTheFileIconsTable->GetSmallImageList()->GetBitmap(filename.GetIconID()));
  }
  else
  {
    m_NotebookWithEditors->SetSelection(m_NotebookWithEditors->GetPageIndex(page));
  }

  return true;
}

bool MDIFrame::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  wxLogTrace("SY_CALL", "+OpenFile");

  if (!filename.GetStat().IsOk())
  {
    wxLogError(_("Cannot open file") + ": " + filename.GetFullPath());
    return false;
  }

  wxExNotebook* notebook = (flags & wxExSTCWithFrame::STC_OPEN_IS_PROJECT
    ? m_NotebookWithProjects : m_NotebookWithEditors);

  wxWindow* page = notebook->GetPageByKey(filename.GetFullPath());

  if (page != NULL)
  {
    notebook->SetSelection(notebook->GetPageIndex(page));
  }

  if (flags & wxExSTCWithFrame::STC_OPEN_IS_PROJECT)
  {
    if (page == NULL)
    {
      wxExListViewFile* project = new wxExListViewFile(m_NotebookWithProjects,
        this,
        filename.GetFullPath(),
        wxID_ANY,
        wxExListViewStandard::LIST_MENU_DEFAULT);

      m_NotebookWithProjects->AddPage(
        project,
        filename.GetFullPath(),
        filename.GetName(),
        true,
        wxArtProvider::GetBitmap(wxART_NORMAL_FILE));
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

    wxExSTCWithFrame* editor = (wxExSTCWithFrame*)page;

    if (page == NULL)
    {
#if wxUSE_CHECKBOX
      if (GetHexModeCheckBox()->GetValue())
        flags |= wxExSTC::STC_OPEN_HEX;
#endif

      wxLogTrace("SY_CALL", "+wxExSTCWithFrame");

      editor = new wxExSTCWithFrame(m_NotebookWithEditors,
        this,
        filename.GetFullPath(),
        line_number,
        match,
        flags);

      wxLogTrace("SY_CALL", "-wxExSTCWithFrame");

      m_NotebookWithEditors->AddPage(
        editor,
        filename.GetFullPath(),
        filename.GetFullName(),
        true,
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(filename.GetIconID()));

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
#if wxUSE_STATUSBAR
    StatusText(wxEmptyString, "PaneItems");
#endif
    break;
  case NOTEBOOK_PROJECTS:
    GetManager().GetPane("PROJECTS").Hide();
    GetManager().Update();
#if wxUSE_STATUSBAR
    StatusText(wxEmptyString, "PaneItems");
#endif
    break;
  default: wxFAIL;
  }
}
