/******************************************************************************\
* File:          frame.cpp
* Purpose:       Implementation of class 'Frame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/config.h>
#include <wx/imaglist.h>
#include <wx/stdpaths.h> // for wxStandardPaths
#include <wx/extension/configdlg.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/header.h>
#include <wx/extension/lexers.h>
#include <wx/extension/otl.h>
#include <wx/extension/printing.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/version.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/stc.h>
#include <wx/extension/report/util.h>
#include "frame.h"
#include "defs.h"
#include "version.h"

class wxExLogStderr : public wxLogStderr
{
  public:
    wxExLogStderr(FILE* fp, Frame* frame) 
      : wxLogStderr(fp)
      , m_Frame(frame) {
#ifdef __WXDEBUG__
        SetVerbose();
#endif        
        SetTimestamp("%x %X");};
  protected:
    virtual void DoLogRecord(
      wxLogLevel level,
      const wxString& msg,
      const wxLogRecordInfo& info)
    {
#ifdef __WXDEBUG__
      m_Frame->SetStatusText(msg);
      
      if (level != wxLOG_Status)
      {
        wxLogStderr::DoLogRecord(level, msg, info);
        m_Frame->Log(level, msg, info);
      }
#else      
      switch (level)
      {
      case wxLOG_Status: 
         m_Frame->SetStatusText(msg); 
         break;
      case wxLOG_Error:
      case wxLOG_Message:
         wxLogStderr::DoLogRecord(level, msg, info);
         m_Frame->Log(level, msg, info);
         break
      }
#endif
    }
  private:
    Frame* m_Frame;
};

BEGIN_EVENT_TABLE(Frame, DecoratedFrame)
  EVT_CLOSE(Frame::OnClose)
  EVT_AUINOTEBOOK_BG_DCLICK(NOTEBOOK_EDITORS, Frame::OnNotebook)
  EVT_MENU(wxID_DELETE, Frame::OnCommand)
  EVT_MENU(wxID_EXECUTE, Frame::OnCommand)
  EVT_MENU(wxID_JUMP_TO, Frame::OnCommand)
  EVT_MENU(wxID_SELECTALL, Frame::OnCommand)
  EVT_MENU(wxID_STOP, Frame::OnCommand)
  EVT_MENU(ID_EDIT_NEXT, Frame::OnCommand)
  EVT_MENU(ID_EDIT_PREVIOUS, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CLOSE, wxID_PREFERENCES, Frame::OnCommand)
  EVT_MENU_RANGE(ID_APPL_LOWEST, ID_APPL_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_ALL_LOWEST, ID_ALL_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_VCS_LOWEST, ID_VCS_HIGHEST, Frame::OnCommand)
  EVT_UPDATE_UI(ID_ALL_STC_CLOSE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_ALL_STC_SAVE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_COPY, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_CUT, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_EXECUTE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_FIND, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_JUMP_TO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PRINT, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PREVIEW, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PASTE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REPLACE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UNDO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REDO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_STOP, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_ADD_HEADER, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_CONTROL_CHAR, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_INSERT_SEQUENCE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_PLAYBACK, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_START_RECORD, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_STOP_RECORD, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_OPTION_VCS, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_PROJECT_SAVE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_FILE_MENU, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_PROJECT_MENU, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SORT_SYNC, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_TOOL_REPORT_REVISION, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(wxID_SAVE, wxID_SAVEAS, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_EDIT_FIND_NEXT, ID_EDIT_FIND_PREVIOUS, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_EDIT_TOGGLE_FOLD, ID_EDIT_UNFOLD_ALL, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(
    ID_OPTION_LIST_SORT_ASCENDING, ID_OPTION_LIST_SORT_TOGGLE, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_PROJECT_OPENTEXT, ID_PROJECT_SAVEAS, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_VIEW_PANE_FIRST + 1, ID_VIEW_PANE_LAST - 1, Frame::OnUpdateUI)
END_EVENT_TABLE()

Frame::Frame(bool open_recent)
  : DecoratedFrame()
  , m_NewFileNo(1)
  , m_NewProjectNo(1)
  , m_History(NULL)
  , m_ProjectWildcard(_("Project Files") + " (*.prj)|*.prj")
  , m_LogFile(wxFileName(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
#else
      wxStandardPaths::Get().GetUserDataDir(),
#endif
      wxTheApp->GetAppName().Lower() + ".log").GetFullPath())
{
  m_OldLog = wxLog::SetActiveTarget(
    new wxExLogStderr(fopen(m_LogFile.c_str() , "a"), this));
    
  const long flag =
    wxAUI_NB_DEFAULT_STYLE |
    wxAUI_NB_CLOSE_ON_ALL_TABS |
    wxAUI_NB_CLOSE_BUTTON |
    wxAUI_NB_WINDOWLIST_BUTTON |
    wxAUI_NB_SCROLL_BUTTONS;

  m_Editors = new wxExNotebook(
    this, 
    this, 
    (wxWindowID)NOTEBOOK_EDITORS, 
    wxDefaultPosition, 
    wxDefaultSize, 
    flag);
    
  m_Lists = new wxExNotebook(
    this, 
    this, 
    (wxWindowID)NOTEBOOK_LISTS, 
    wxDefaultPosition, 
    wxDefaultSize, 
    flag);
    
  m_Projects = new wxExNotebook(
    this, 
    this, 
    (wxWindowID)NOTEBOOK_PROJECTS, 
    wxDefaultPosition, 
    wxDefaultSize, 
    flag);
    
  m_History = new wxExListViewWithFrame(this, this,
    wxExListViewStandard::LIST_HISTORY,
    wxExListViewStandard::LIST_MENU_DEFAULT);
    
  m_DirCtrl = new wxExGenericDirCtrl(this, this);
    
  wxExSTC* asciiTable = new wxExSTC(this);
  AddAsciiTable(asciiTable);

  GetManager().AddPane(m_Editors, wxAuiPaneInfo()
    .CenterPane()
    .MaximizeButton(true)
    .Name("FILES")
    .Caption(_("Files")));

  GetManager().AddPane(m_Projects, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .Name("PROJECTS")
    .MinSize(wxSize(150, -1))
    .Caption(_("Projects")));

  GetManager().AddPane(m_DirCtrl, wxAuiPaneInfo()
    .Left()
    .Name("DIRCTRL")
    .Caption(_("Explorer")));

  GetManager().AddPane(asciiTable, wxAuiPaneInfo()
    .Left()
    .Name("ASCIITABLE")
    .Caption(_("Ascii Table")));

  GetManager().AddPane(m_History, wxAuiPaneInfo()
    .Left().Name("HISTORY")
    .Caption(_("History")));

  GetManager().AddPane(m_Lists, wxAuiPaneInfo()
    .Bottom()
    .MaximizeButton(true)
    .MinSize(250, 100).Name("OUTPUT")
    .Caption(_("Output")));

  const wxString perspective = wxConfigBase::Get()->Read("Perspective");

  if (perspective.empty())
  {
    GetManager().GetPane("ASCIITABLE").Hide();
    GetManager().GetPane("DIRCTRL").Hide();
    GetManager().GetPane("HISTORY").Hide();
    GetManager().GetPane("LOG").Hide();
    GetManager().GetPane("PROJECTS").Hide();
  }
  else
  {
    GetManager().LoadPerspective(perspective);
  }

  // Regardless of the perspective initially hide the next panels.
  GetManager().GetPane("OUTPUT").Hide();
  
  HideViBar();
  
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
          WIN_IS_PROJECT);
      }
      else
      {
        GetManager().GetPane("PROJECTS").Hide();
      }
    }
  }
  else
  {
    GetManager().GetPane("PROJECTS").Hide();
  }
  
  StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
  
  // End with update, so all changes in the manager are handled.
  GetManager().Update();
}

Frame::~Frame()
{
  delete m_OldLog;
}

wxExListViewStandard* Frame::Activate(
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

void Frame::AddAsciiTable(wxExSTC* stc)
{
  // Do not show an edge, eol or whitespace for ascii table.
  stc->SetEdgeMode(wxSTC_EDGE_NONE);
  stc->SetViewEOL(false);
  stc->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  // And override tab width.
  stc->SetTabWidth(5);

  for (auto i = 1; i <= 255; i++)
  {
    stc->AddText(wxString::Format("%d\t%c", i, (wxUniChar)i));
    stc->AddText((i % 5 == 0) ? stc->GetEOL(): "\t");
  }

  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
  stc->SetReadOnly(true);
}

void Frame::AddHeader(wxExSTC* stc)
{
  const wxExHeader header;

  if (header.ShowDialog(this) != wxID_CANCEL)
  {
    if (stc->GetLexer().GetScintillaLexer() == "hypertext")
    {
      stc->GotoLine(1);
    }
    else
    {
      stc->DocumentStart();
    }

    stc->AddText(header.Get(&stc->GetFile().GetFileName()));
  }
}

wxExListViewWithFrame* Frame::AddPage(
  wxExListViewStandard::ListType type, 
  const wxExLexer* lexer)
{
  const wxString name = wxExListViewStandard::GetTypeDescription(type) +
    (lexer != NULL ?  " " + lexer->GetScintillaLexer(): wxString(wxEmptyString));

  wxExListViewWithFrame* list = 
    (wxExListViewWithFrame*)m_Lists->GetPageByKey(name);

  if (list == NULL && type != wxExListViewStandard::LIST_FILE)
  {
    list = new wxExListViewWithFrame(
      m_Lists, this,
      type,
      wxID_ANY,
      wxExListViewStandard::LIST_MENU_DEFAULT,
      lexer);

    m_Lists->AddPage(list, name, name, true);
  }

  return list;
}

bool Frame::AllowCloseAll(wxWindowID id)
{
  switch (id)
  {
  case NOTEBOOK_EDITORS: 
    return m_Editors->ForEach(ID_ALL_STC_CLOSE); 
    break;
  case NOTEBOOK_PROJECTS: 
    return wxExForEach(m_Projects, ID_LIST_ALL_CLOSE); 
    break;
  default:
    wxFAIL;
    break;
  }

  return true;
}

bool Frame::DialogProjectOpen()
{
  wxFileDialog dlg(this,
    _("Select Projects"),
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
#else
      wxStandardPaths::Get().GetUserDataDir(),
#endif
    wxEmptyString,
    m_ProjectWildcard,
    wxFD_OPEN | wxFD_MULTIPLE);

  if (dlg.ShowModal() == wxID_CANCEL) return false;

  wxArrayString files;
  dlg.GetPaths(files);
  wxExOpenFiles(this, files, WIN_IS_PROJECT);

  return true;
}

wxExListView* Frame::GetListView()
{
  if (m_History->HasFocus())
  {
    return m_History;
  }
  else if (GetProject() != NULL && GetProject()->HasFocus())
  {
    return GetProject();
  }
  else if (
    !m_Lists->IsShown() || 
     m_Lists->GetPageCount() == 0)
  {
    return NULL;
  }
  else
  {
    return (wxExListView*)m_Lists->GetPage(
      m_Lists->GetSelection());
  }
}

wxExListViewFile* Frame::GetProject()
{
  if (
    !m_Projects->IsShown() || 
     m_Projects->GetPageCount() == 0)
  {
    return NULL;
  }
  else
  {
    return (wxExListViewFile*)m_Projects->
      GetPage(m_Projects->GetSelection());
  }
}

wxExSTC* Frame::GetSTC()
{
  if (
    !m_Editors->IsShown() || 
     m_Editors->GetPageCount() == 0)
  {
    return NULL;
  }
  else
  {
    return (wxExSTC*)m_Editors->GetPage(
      m_Editors->GetSelection());
  }
}

void Frame::Log(
  wxLogLevel level,
  const wxString& msg,
  const wxLogRecordInfo& info)
{
  m_LogText += 
    wxDateTime(info.timestamp).Format(wxLog::GetTimestamp()) + " " + 
    msg + "\n";
}

void Frame::NewFile(bool as_project)
{
  const wxString name = (as_project ? _("project") : _("textfile"));
  const auto use_no = (as_project ? m_NewProjectNo : m_NewFileNo);
  const wxString text = wxString::Format("%s%d", name.c_str(), use_no);
  wxString key;
  auto* notebook = (as_project ? m_Projects : m_Editors);
  wxWindow* page;

  if (as_project)
  {
    const wxFileName fn(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
#else
      wxStandardPaths::Get().GetUserDataDir(),
#endif
      text + ".prj");

    key = fn.GetFullPath();

    page = new wxExListViewFile(notebook,
      this,
      key,
      wxID_ANY,
      wxExListViewStandard::LIST_MENU_DEFAULT);

    ((wxExListViewFile*)page)->FileNew(key);

    m_NewProjectNo++;
  }
  else
  {
    if (wxConfigBase::Get()->ReadBool("HexMode", false))
    {
      // In hex mode we cannot edit the file.
      return;
    }

    key = text;
    page = new wxExSTCWithFrame(notebook, this);

    ((wxExSTC*)page)->GetFile().FileNew(text);

    m_NewFileNo++;
  }

  // This file does yet exist, so do not give it a bitmap.
  notebook->AddPage(
    page,
    key,
    text,
    true);

  GetManager().GetPane(as_project ? "PROJECTS" : "FILES").Show();
  GetManager().Update();
}

void Frame::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    if (!AllowCloseAll(NOTEBOOK_PROJECTS) || !AllowCloseAll(NOTEBOOK_EDITORS))
    {
      event.Veto();
      return;
    }
  }

  delete wxLog::SetActiveTarget(NULL);
  
  wxConfigBase::Get()->Write("Perspective", GetManager().SavePerspective());

  event.Skip();
}

void Frame::OnCommand(wxCommandEvent& event)
{
  auto* editor = GetSTC();
  auto* project = GetProject();

  // VCS commands.
  if (event.GetId() > ID_VCS_LOWEST && 
      event.GetId() < ID_VCS_HIGHEST)
  {
    wxExVCS(wxArrayString(), event.GetId()).Request(this);
  }
  // edit commands
  // Do not change the wxID* in wxID_LOWEST and wdID_HIGHEST,
  // as wxID_ABOUT etc. is used here and not in the editor.
  // That causes appl to hang.
  else if ((
       event.GetId() == wxID_UNDO ||
       event.GetId() == wxID_REDO ||
       event.GetId() == wxID_DELETE ||
       event.GetId() == wxID_SELECTALL ||
       event.GetId() == wxID_JUMP_TO) ||
      (event.GetId() >= wxID_CUT && event.GetId() <= wxID_CLEAR) ||
      (event.GetId() >= ID_EDIT_STC_LOWEST && event.GetId() <= ID_EDIT_STC_HIGHEST))
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
    info.AddArtist("http://tango.freedesktop.org/Tango_Desktop_Project");

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
    info.SetCopyright("(c) 1998-2011, Anton van Wezenbeek. " 
      + wxString(_("All rights reserved.")));
    info.SetWebSite("http://syncped.1.xpdev-hosted.com/index.htm");
      
    wxAboutBox(info);
    }
    break;
  case wxID_CLOSE:
    if (editor != NULL)
    {
      if (!AllowClose(
        m_Editors->GetId(),
        editor))
      {
        return;
      }
        
      m_Editors->DeletePage(editor->GetFileName().GetFullPath());
    }
    break;
    
  case wxID_EXIT: Close(true); break;
  
  case wxID_HELP:
    wxLaunchDefaultBrowser(wxString("http://syncped.1.xpdev-hosted.com/tags/v") + 
      APPL_VERSION + wxString("/syncped.htm"));
    break;
    
  case wxID_NEW: NewFile(); break;
  
  case wxID_PREVIEW:
    if (GetSTC() != NULL)
    {
      GetSTC()->PrintPreview();
    }
    else if (GetListView() != NULL)
    {
      GetListView()->PrintPreview();
    }
    break;
  case wxID_PRINT:
    if (GetSTC() != NULL)
    {
      GetSTC()->Print();
    }
    else if (GetListView() != NULL)
    {
      GetListView()->Print();
    }
    break;
  case wxID_PRINT_SETUP:
    wxExPrinting::Get()->GetHtmlPrinter()->PageSetup();
    break;

  case wxID_SAVE:
    if (editor != NULL)
    {
      editor->GetFile().FileSave();

      if (editor->GetFileName() == wxExLexers::Get()->GetFileName())
      {
        wxExLexers::Get()->Read();
        m_Editors->ForEach(ID_ALL_STC_SET_LEXER);

        // As the lexer might have changed, update status bar field as well.
#if wxUSE_STATUSBAR
        UpdateStatusBar(editor, "PaneLexer");
#endif
      }
      else if (editor->GetFileName() == wxExVCS::GetFileName())
      {
        wxExVCS::Read();
      }
    }
    break;
  case wxID_SAVEAS:
    if (editor != NULL)
    {
      const wxString old_key = editor->GetFileName().GetFullPath();

      if (!event.GetString().empty())
      {
        editor->GetFile().FileSave(event.GetString());
      }
      else
      {
        wxExFileDialog dlg(
          this, 
          &editor->GetFile(), 
          _("File Save As"), 
          wxFileSelectorDefaultWildcardStr, 
          wxFD_SAVE);

        if (dlg.ShowModal() != wxID_OK)
        {
          return;
        }

        editor->GetFile().FileSave(dlg.GetPath());
      }

      m_Editors->SetPageText(
        old_key,
        editor->GetFileName().GetFullPath(),
        editor->GetFileName().GetFullName(),
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wxExGetIconID(editor->GetFileName())));
    }
    editor->PropertiesMessage();
    break;

  case wxID_EXECUTE: ProcessRun(); break;
  case wxID_STOP: ProcessStop(); break;

  case ID_ALL_STC_CLOSE:
  case ID_ALL_STC_SAVE:
    m_Editors->ForEach(event.GetId());
    break;

  case ID_EDIT_ADD_HEADER: if (editor != NULL) AddHeader(editor); break;
  case ID_EDIT_INSERT_SEQUENCE: if (editor != NULL) SequenceDialog(editor); break;

  case ID_EDIT_MACRO_PLAYBACK: if (editor != NULL) editor->MacroPlayback(); break;
  case ID_EDIT_MACRO_START_RECORD: if (editor != NULL) editor->StartRecord(); break;
  case ID_EDIT_MACRO_STOP_RECORD: if (editor != NULL) editor->StopRecord(); break;
  
  case ID_EDIT_NEXT:
    if (m_Editors->GetSelection() == 
        m_Editors->GetPageCount() - 1)
    {
      m_Editors->SetSelection(0);
    }
    else
    {
      m_Editors->AdvanceSelection();
    }
    break;
  case ID_EDIT_PREVIOUS:
    if (m_Editors->GetSelection() == 0)
    {
      m_Editors->SetSelection(
        m_Editors->GetPageCount() - 1);
    }
    else
    {
      m_Editors->AdvanceSelection(false);
    }
    break;

  case ID_OPTION_EDITOR:
    wxExSTC::ConfigDialog(this,
      _("Editor Options"),
      wxExSTC::STC_CONFIG_MODELESS | wxExSTC::STC_CONFIG_WITH_APPLY,
      event.GetId());
    break;
    
  case ID_OPTION_LIST_COMPARATOR: 
    {
      std::vector<wxExConfigItem> v;
      v.push_back(wxExConfigItem(_("Comparator"), CONFIG_FILEPICKERCTRL));
      wxExConfigDialog(
        this,
        v,
        _("Set List Comparator")).ShowModal();
    }
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

        wxExForEach(m_Projects, ID_LIST_ALL_ITEMS, font);
        wxExForEach(m_Lists, ID_LIST_ALL_ITEMS, font);
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

  case ID_OPTION_VCS: 
    if (wxExVCS().ConfigDialog(this) == wxID_OK)
    { 
      wxExVCS vcs;
      StatusText(vcs.GetEntry().GetName(), "PaneVCS");
    }
    break;
    
  case ID_PROCESS_SELECT: ProcessConfigDialog(this); break;

  case ID_PROJECT_CLOSE:
    if (project != NULL)
    {
      m_Projects->DeletePage(project->GetFileName().GetFullPath());
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
        OpenFile(project->GetFileName());
      }
    }
    break;
  case ID_PROJECT_SAVEAS:
    if (project != NULL)
    {
      const wxString old_key = m_Projects->GetKeyByPage(project);

      wxExFileDialog dlg(
        this, project, 
        _("Project Save As"), 
        m_ProjectWildcard, 
        wxFD_SAVE);

      if (dlg.ShowModal() == wxID_OK)
      {
        project->FileSave(dlg.GetPath());

        m_Projects->SetPageText(
          old_key,
          project->GetFileName().GetFullPath(),
          project->GetFileName().GetName());
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

    // Place new page before page for editor.
    m_Editors->InsertPage(
      m_Editors->GetPageIndex(editor),
      stc,
      // key should be unique
      wxString::Format("stc%d", stc->GetId()),
      stc->GetFileName().GetFullName(),
      true,
      wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
        wxExGetIconID(stc->GetFileName())));

    stc->SetDocPointer(editor->GetDocPointer());
    }
    break;
    
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
    
  case ID_VIEW_HISTORY: 
    TogglePane("HISTORY");
#if wxUSE_STATUSBAR
    UpdateStatusBar(m_History);
#endif
    break;
  case ID_VIEW_OUTPUT: 
    TogglePane("OUTPUT");
    break;
  case ID_VIEW_PROJECTS: 
    if (m_Projects->GetPageCount() == 0)
    {
      NewFile(true);
    }
    else
    {
      TogglePane("PROJECTS");
    }
    break;
    
  default: 
    wxFAIL;
    break;
  }
}

void Frame::OnCommandConfigDialog(
  wxWindowID dialogid,
  int commandid)
{
  if (dialogid == ID_OPTION_EDITOR)
  {
    if (commandid != wxID_CANCEL)
    {
      m_Editors->ForEach(ID_ALL_STC_CONFIG_GET);
    }
  }
  else
  {
    DecoratedFrame::OnCommandConfigDialog(dialogid, commandid);
  }
}

void Frame::OnNotebook(wxAuiNotebookEvent& event)
{
  FileHistoryPopupMenu();
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
    case wxID_EXECUTE: 
      event.Enable( ProcessIsSelected() &&
                   !ProcessIsRunning()); 
      break;
    case wxID_STOP: event.Enable(ProcessIsRunning()); break;
    case wxID_PREVIEW:
    case wxID_PRINT:
      event.Enable(
        (GetSTC() != NULL && GetSTC()->GetLength() > 0) ||
        (GetListView() != NULL && GetListView()->GetItemCount() > 0));
      break;

    case ID_ALL_STC_CLOSE:
    case ID_ALL_STC_SAVE:
      event.Enable(m_Editors->GetPageCount() > 2);
    break;

    case ID_OPTION_LIST_SORT_ASCENDING:
    case ID_OPTION_LIST_SORT_DESCENDING:
    case ID_OPTION_LIST_SORT_TOGGLE:
      event.Check(
        event.GetId() - ID_OPTION_LIST_SORT_ASCENDING == 
        wxConfigBase::Get()->ReadLong("List/SortMethod", SORT_TOGGLE) - SORT_ASCENDING);
    break;
    
    case ID_OPTION_VCS:
      event.Enable(wxExVCS::GetCount() > 0);
      break;

    case ID_PROJECT_CLOSE:
    case ID_PROJECT_SAVEAS:
      event.Enable(GetProject() != NULL && GetProject()->IsShown());
      break;
    case ID_PROJECT_OPENTEXT:
      event.Enable(
        GetProject() != NULL && !GetProject()->GetFileName().GetFullPath().empty());
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

    case ID_TOOL_REPORT_REVISION:
      event.Check(!wxExVCS().Use());
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
    case ID_VIEW_HISTORY:
      event.Check(GetManager().GetPane("HISTORY").IsShown());
      break;
    case ID_VIEW_OUTPUT:
      event.Check(GetManager().GetPane("OUTPUT").IsShown());
      break;
    case ID_VIEW_PROJECTS:
      event.Check(GetManager().GetPane("PROJECTS").IsShown());
      break;

    default:
    {
      auto* editor = GetSTC();
      auto* list = (wxExListViewFile*)GetListView();

      if (list == NULL && editor != NULL && editor->IsShown())
      {
        event.Enable(true);

        switch (event.GetId())
        {
        case wxID_FIND:
        case wxID_JUMP_TO:
        case wxID_REPLACE:
        case wxID_SAVEAS:
        case ID_EDIT_FIND_NEXT:
        case ID_EDIT_FIND_PREVIOUS:
        case ID_EDIT_FOLD_ALL:
        case ID_EDIT_UNFOLD_ALL:
          event.Enable(editor->GetLength() > 0);
          break;

        case ID_EDIT_MACRO_PLAYBACK:
          event.Enable(editor->MacroIsRecorded() && !editor->MacroIsRecording());
          break;
        case ID_EDIT_MACRO_START_RECORD:
          event.Enable(
            editor->GetLength() > 0 && !editor->MacroIsRecording() && 
              (!editor->GetVi().GetIsActive() || 
               (editor->GetVi().GetIsActive() && editor->GetVi().GetInsertMode())));
          break;
        case ID_EDIT_MACRO_STOP_RECORD:
          event.Enable(editor->MacroIsRecording());
          break;

        case ID_EDIT_TOGGLE_FOLD:
          event.Enable(
            editor->GetTextLength() > 0 &&
            editor->GetFoldLevel(editor->GetCurrentLine()) > wxSTC_FOLDLEVELBASE);
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
          
        case wxID_COPY:
          event.Enable(!editor->GetSelectedText().empty());
          break;
        case wxID_CUT:
          event.Enable(!editor->GetSelectedText().empty() && !editor->GetReadOnly());
          break;

        case ID_EDIT_CONTROL_CHAR:
          if (editor->GetReadOnly() && editor->GetSelectedText().length() != 1)
          {
            event.Enable(false);
          }
          break;

        case ID_EDIT_ADD_HEADER:
        case ID_EDIT_INSERT_SEQUENCE:
          event.Enable(!editor->GetReadOnly());
          break;
  
        default:
          wxFAIL;
        }
      }
      else if (list != NULL)
      {
        event.Enable(false);

        if (
          event.GetId() > ID_TOOL_LOWEST &&
          event.GetId() < ID_TOOL_HIGHEST)
        {
          event.Enable(list->GetSelectedItemCount() > 0);
        }
        else
        {
          switch (event.GetId())
          {
          case wxID_COPY:
          case wxID_CUT:
            event.Enable(list->GetSelectedItemCount() > 0);
            break;

          case wxID_FIND:
            event.Enable(list->GetItemCount() > 0);
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
      else
      {
        event.Enable(false);
      }
    }
  }
}

bool Frame::OpenFile(
  const wxExFileName& filename,
  const wxExVCSEntry& vcs,
  long flags)
{
  const wxString unique = 
    vcs.GetCommand().GetCommand() + " " + vcs.GetFlags();
    
  const wxString key = filename.GetFullPath() + unique;

  auto* page = m_Editors->SelectPage(key);

  if (page == NULL)
  {
    wxExSTCWithFrame* editor = new wxExSTCWithFrame(
      m_Editors, 
      this,
      vcs.GetOutput(),
      flags,
      filename.GetFullName() + " " + unique);

    wxExVCSCommandOnSTC(
      vcs.GetCommand(), filename.GetLexer(), editor);
    
    // Place new page before the one used for vcs.
    m_Editors->InsertPage(
      m_Editors->GetPageIndexByKey(filename.GetFullPath()),
      editor,
      key,
      filename.GetFullName() + " " + unique,
      true);
  }

  return true;
}

bool Frame::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  if (!filename.GetStat().IsOk())
  {
    wxLogError(_("Cannot open file") + ": " + filename.GetFullPath());
    return false;
  }
  
  auto* notebook = (flags & WIN_IS_PROJECT ? m_Projects : m_Editors);

  wxWindow* page = notebook->SelectPage(filename.GetFullPath());

  if (flags & WIN_IS_PROJECT)
  {
    if (page == NULL)
    {
      wxExListViewFile* project = new wxExListViewFile(m_Projects,
        this,
        filename.GetFullPath(),
        wxID_ANY,
        wxExListViewStandard::LIST_MENU_DEFAULT);

      notebook->AddPage(
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
      if (wxConfigBase::Get()->ReadBool("HexMode", false))
        flags |= wxExSTC::STC_WIN_HEX;

      editor = new wxExSTCWithFrame(m_Editors,
        this,
        filename,
        line_number,
        match,
        flags);

      notebook->AddPage(
        editor,
        filename.GetFullPath(),
        filename.GetFullName(),
        true,
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wxExGetIconID(filename)));

      if (GetManager().GetPane("DIRCTRL").IsShown())
      {
        m_DirCtrl->SelectPath(filename.GetFullPath());
      }
      
      if (filename.GetFullPath() == m_LogFile)
      {
        editor->DocumentEnd();
      }
      
      // Do not show an edge for project files opened as text.
      if (filename.GetExt() == "prj")
      {
        editor->SetEdgeMode(wxSTC_EDGE_NONE);
      }
    }
    else if (line_number > 0)
    {
      editor->GotoLineAndSelect(line_number, match);
    }
  }

  return true;
}

void Frame::SequenceDialog(wxExSTC* stc)
{
  static wxString start_previous;

  const wxString start = wxGetTextFromUser(
    _("Input") + ":",
    _("Start Of Sequence"),
    start_previous,
    this);

  if (start.empty()) return;

  start_previous = start;

  static wxString end_previous = start;

  const wxString end = wxGetTextFromUser(
    _("Input") + ":",
    _("End Of Sequence"),
    end_previous,
    this);

  if (end.empty()) return;

  end_previous = end;

  if (start.length() != end.length())
  {
    wxLogStatus(_("Start and end sequence should have same length"));
    return;
  }

  long lines = 1;

  for (int pos = end.length() - 1; pos >= 0; pos--)
  {
    lines *= abs(end[pos] - start[pos]) + 1;
  }

  if (wxMessageBox(wxString::Format(_("Generate %ld lines"), lines) + "?",
    _("Confirm"),
    wxOK | wxCANCEL | wxICON_QUESTION) == wxCANCEL)
  {
    return;
  }

  wxBusyCursor wait;

  wxString sequence = start;

  long actual_line = 0;

  while (sequence != end)
  {
    stc->AddText(sequence + stc->GetEOL());
    actual_line++;

    if (actual_line > lines)
    {
      wxFAIL;
      return;
    }

    if (start < end)
    {
      sequence.Last() = (int)sequence.Last() + 1;
    }
    else
    {
      sequence.Last() = (int)sequence.Last() - 1;
    }

    for (int pos = end.length() - 1; pos > 0; pos--)
    {
      if (start < end)
      {
        if (sequence[pos] > end[pos])
        {
          sequence[pos - 1] = (int)sequence[pos - 1] + 1;
          sequence[pos] = start[pos];
        }
      }
      else
      {
        if (sequence[pos] < end[pos])
        {
          sequence[pos - 1] = (int)sequence[pos - 1] - 1;
          sequence[pos] = start[pos];
        }
      }
    }
  }

  stc->AddText(sequence + stc->GetEOL());
}

void Frame::StatusBarDoubleClicked(const wxString& pane)
{
  if (pane.empty())
  {
    wxExSTCWithFrame* editor;
    
    if ((editor = (wxExSTCWithFrame*)m_Editors->SelectPage("LOGTAIL")) == NULL)
    {
      editor = new wxExSTCWithFrame(m_Editors, this);
      editor->SetName(_("Log"));
      editor->SetEdgeMode(wxSTC_EDGE_NONE);
      editor->SetReadOnly(true); // to update page title
      
      m_Editors->AddPage(editor, "LOGTAIL", _("Log"), true);
    }
    
    editor->SetText(m_LogText);
    editor->EmptyUndoBuffer();
    editor->SetSavePoint();
    editor->DocumentEnd();
    editor->SetReadOnly(true);
  }
  else if (pane == "PaneTheme")
  {
    if (wxExLexers::Get()->ShowThemeDialog(this))
    {
      StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
      m_Editors->ForEach(ID_ALL_STC_SET_LEXER_THEME);
    }
  }
  else if (pane == "PaneVCS")
  {
    if (wxExVCS::GetCount() > 0)
    {
      wxExMenu* menu = new wxExMenu;
      
      if (menu->AppendVCS())
      {
        PopupMenu(menu);
      }
      
      delete menu;
    }
  }
  else
  {
    DecoratedFrame::StatusBarDoubleClicked(pane);
  }
}

void Frame::StatusBarDoubleClickedRight(const wxString& pane)
{
  if (pane.empty())
  {
    OpenFile(m_LogFile);
  }
  else if (pane == "PaneLexer" || pane == "PaneTheme")
  {
    OpenFile(wxExLexers::Get()->GetFileName());
  }
  else if (pane == "PaneVCS")
  {
    OpenFile(wxExVCS::GetFileName());
  }
  else
  {
    DecoratedFrame::StatusBarDoubleClickedRight(pane);
  }
}

void Frame::SyncCloseAll(wxWindowID id)
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
