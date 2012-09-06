////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of class Frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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

BEGIN_EVENT_TABLE(Frame, DecoratedFrame)
  EVT_CLOSE(Frame::OnClose)
  EVT_AUINOTEBOOK_BG_DCLICK(NOTEBOOK_EDITORS, Frame::OnNotebook)
  EVT_MENU(wxID_DELETE, Frame::OnCommand)
  EVT_MENU(wxID_EXECUTE, Frame::OnCommand)
  EVT_MENU(wxID_JUMP_TO, Frame::OnCommand)
  EVT_MENU(wxID_SELECTALL, Frame::OnCommand)
  EVT_MENU(wxID_STOP, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CLOSE, wxID_PREFERENCES, Frame::OnCommand)
  EVT_MENU_RANGE(ID_APPL_LOWEST, ID_APPL_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_ALL_LOWEST, ID_ALL_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_VCS_LOWEST, ID_VCS_HIGHEST, Frame::OnCommand)
  EVT_UPDATE_UI(ID_ALL_STC_CLOSE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_ALL_STC_SAVE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_EXECUTE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_JUMP_TO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PRINT, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PREVIEW, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REPLACE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UNDO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REDO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_STOP, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_CONTROL_CHAR, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_PLAYBACK, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_START_RECORD, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_STOP_RECORD, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_OPTION_VCS, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_PROJECT_SAVE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_FILE_MENU, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_PROJECT_MENU, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SORT_SYNC, Frame::OnUpdateUI)
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
  , m_IsClosing(false)
  , m_NewFileNo(1)
  , m_NewProjectNo(1)
  , m_SplitId(1)
  , m_DirCtrl(NULL)
  , m_Editors(NULL)
  , m_History(NULL)
  , m_Lists(NULL)
  , m_Process(new wxExProcessListView(this))
  , m_Projects(NULL)
  , m_PaneFlag(
    wxAUI_NB_DEFAULT_STYLE |
    wxAUI_NB_CLOSE_ON_ALL_TABS |
    wxAUI_NB_CLOSE_BUTTON |
    wxAUI_NB_WINDOWLIST_BUTTON |
    wxAUI_NB_SCROLL_BUTTONS)
  , m_ProjectWildcard(_("Project Files") + " (*.prj)|*.prj")
{
  wxExViMacros::LoadDocument();

  m_Editors = new wxExNotebook(
    this, 
    this, 
    (wxWindowID)NOTEBOOK_EDITORS, 
    wxDefaultPosition, 
    wxDefaultSize, 
    m_PaneFlag);
    
  m_Lists = new wxExNotebook(
    this, 
    this, 
    (wxWindowID)NOTEBOOK_LISTS, 
    wxDefaultPosition, 
    wxDefaultSize, 
    m_PaneFlag);
    
  m_DirCtrl = new wxExGenericDirCtrl(this, this);
    
  wxExSTC* asciiTable = new wxExSTC(this);
  AddAsciiTable(asciiTable);

  GetManager().AddPane(m_Editors, wxAuiPaneInfo()
    .CenterPane()
    .MaximizeButton(true)
    .Name("FILES")
    .Caption(_("Files")));

//  if (wxConfigBase::Get()->ReadBool("ShowProjects", false))
  {
    AddPaneProjects();
  }

  GetManager().AddPane(m_DirCtrl, wxAuiPaneInfo()
    .Left()
    .Name("DIRCTRL")
    .Caption(_("Explorer")));

  GetManager().AddPane(asciiTable, wxAuiPaneInfo()
    .Left()
    .Name("ASCIITABLE")
    .Caption(_("Ascii Table")));

  GetManager().AddPane(m_Lists, wxAuiPaneInfo()
    .Bottom()
    .MaximizeButton(true)
    .MinSize(250, 100).Name("OUTPUT")
    .Row(0)
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
  
  if (wxConfigBase::Get()->ReadBool("ShowHistory", false))
  {
    AddPaneHistory();
  }

  // Regardless of the perspective initially hide the next panels.
  GetManager().GetPane("OUTPUT").Hide();
  
  HideExBar();
  
  if (open_recent)
  {
    if (!GetRecentFile().empty())
    {
      long count;
      
      if (wxConfigBase::Get()->Read("OpenFiles", &count))
      {
        wxArrayString ar;
        
        if (count > 0)
        {
          for (int i = count - 1; i >= 0; i--)
          {
            ar.Add(GetFileHistory().GetHistoryFile(i));
          }  
  
          wxExOpenFiles(this, ar);
        }
      }
      else
      {
        OpenFile(wxExFileName(GetRecentFile()));
      }
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

wxExListViewFileName* Frame::Activate(
  wxExListViewFileName::wxExListType type, 
  const wxExLexer* lexer)
{
  if (type == wxExListViewFileName::LIST_FILE)
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

  for (int i = 1; i <= 255; i++)
  {
    switch (i)
    {
      case  9: stc->AddText(wxString::Format("%3d\tTAB", i)); break;
      case 10: stc->AddText(wxString::Format("%3d\tLF", i)); break;
      case 13: stc->AddText(wxString::Format("%3d\tCR", i)); break;
        
      default:
        stc->AddText(wxString::Format("%3d\t%c", i, (wxUniChar)i));
    }
    
    stc->AddText((i % 5 == 0) ? stc->GetEOL(): "\t");
  }

  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
  stc->SetReadOnly(true);
}

wxExListViewWithFrame* Frame::AddPage(
  wxExListViewFileName::wxExListType type, 
  const wxExLexer* lexer)
{
  const wxString name = wxExListViewFileName::GetTypeDescription(type) +
    (lexer != NULL ?  " " + lexer->GetDisplayLexer(): wxString(wxEmptyString));

  wxExListViewWithFrame* list = 
    (wxExListViewWithFrame*)m_Lists->GetPageByKey(name);

  if (list == NULL && type != wxExListViewFileName::LIST_FILE)
  {
    list = new wxExListViewWithFrame(
      m_Lists, this,
      type,
      wxID_ANY,
      wxExListViewWithFrame::LIST_MENU_DEFAULT,
      lexer);

    m_Lists->AddPage(list, name, name, true);
  }

  return list;
}

void Frame::AddPaneHistory()
{
  m_History = new wxExListViewWithFrame(this, this,
    wxExListViewFileName::LIST_HISTORY,
    wxExListViewWithFrame::LIST_MENU_DEFAULT);
        
  GetManager().AddPane(m_History, wxAuiPaneInfo()
    .Left().Name("HISTORY")
    .Caption(_("History")));
}
       
void Frame::AddPaneProjects()
{
  m_Projects = new wxExNotebook(
    this, 
    this, 
    (wxWindowID)NOTEBOOK_PROJECTS, 
    wxDefaultPosition, 
    wxDefaultSize, 
    m_PaneFlag);
    
  GetManager().AddPane(m_Projects, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .Name("PROJECTS")
    .MinSize(wxSize(150, -1))
    .Caption(_("Projects")));
}

bool Frame::AllowCloseAll(wxWindowID id)
{
  switch (id)
  {
  case NOTEBOOK_EDITORS: 
    return m_Editors->ForEach(ID_ALL_STC_CLOSE); 
    break;
  case NOTEBOOK_PROJECTS: 
    if (m_Projects != NULL)
    {
      return wxExForEach(m_Projects, ID_LIST_ALL_CLOSE); 
    }
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

wxExSTC* Frame::ExecExCommand(int command)
{
  switch (command)
  {
  case ID_EDIT_NEXT:
    if (m_Editors->GetSelection() == 
        m_Editors->GetPageCount() - 1)
    {
      return NULL;
    }
    else
    {
      m_Editors->AdvanceSelection();
    }
    break;
    
  case ID_EDIT_PREVIOUS:
    if (m_Editors->GetSelection() == 0)
    {
      return NULL;
    }
    else
    {
      m_Editors->AdvanceSelection(false);
    }
    break;
    
  default:
    wxFAIL;
  }
  
  return (wxExSTC*)m_Editors->
    GetPage(m_Editors->GetSelection());
}

wxExListViewFile* Frame::GetProject()
{
  if 
    (m_Projects == NULL ||
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

void Frame::NewFile(bool as_project)
{
  if (as_project && m_Projects == NULL)
  {
    AddPaneProjects();
  }
  
  const wxString name = (as_project ? _("project") : _("textfile"));
  const int use_no = (as_project ? m_NewProjectNo : m_NewFileNo);
  const wxString text = wxString::Format("%s%d", name.c_str(), use_no);
  wxString key;
  wxExNotebook* notebook = (as_project ? m_Projects : m_Editors);
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
      wxExListViewWithFrame::LIST_MENU_DEFAULT);

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
  m_IsClosing = true; 
  const long count = m_Editors->GetPageCount();
  
  if (event.CanVeto())
  {
    if (
       m_Process->IsRunning() || 
      !AllowCloseAll(NOTEBOOK_PROJECTS) || 
      !AllowCloseAll(NOTEBOOK_EDITORS))
    {
      event.Veto();
      
      if (m_Process->IsRunning())
      {
        wxLogStatus(_("Process is running"));
      }
      
      m_IsClosing = false;
      
      return;
    }
  }
  
  wxExViMacros::SaveDocument();

  wxConfigBase::Get()->Write("Perspective", GetManager().SavePerspective());
  wxConfigBase::Get()->Write("OpenFiles", count);
  wxConfigBase::Get()->Write("ShowHistory", 
    m_History != NULL && m_History->IsShown());
  wxConfigBase::Get()->Write("ShowProjects", 
    m_Projects != NULL && m_Projects->IsShown());

  wxDELETE(m_Process);
  
  event.Skip();
}

void Frame::OnCommand(wxCommandEvent& event)
{
  wxExSTC* editor = GetSTC();
  wxExListViewFile* project = GetProject();

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
    info.SetVersion(wxExGetVersionInfo().GetVersionOnlyString());

#ifdef wxExUSE_EMBEDDED_SQL
    info.AddDeveloper(wxExOTL::VersionInfo().GetVersionString());
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
    info.SetCopyright(wxExGetVersionInfo().GetCopyright());
    info.SetWebSite("http://antonvw.github.com/syncped/index.htm");
      
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
    wxLaunchDefaultBrowser("http://antonvw.github.com/syncped/syncped.htm");
    break;
    
  case wxID_NEW: NewFile(); break;
  
  case wxID_PREVIEW:
    if (editor != NULL)
    {
      editor->PrintPreview();
    }
    else if (GetListView() != NULL)
    {
      GetListView()->PrintPreview();
    }
    break;
  case wxID_PRINT:
    if (editor != NULL)
    {
      editor->Print();
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
      else if (editor->GetFileName() == wxExViMacros::GetFileName())
      {
        wxExViMacros::LoadDocument();
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
          wxGetStockLabel(wxID_SAVEAS), 
          wxFileSelectorDefaultWildcardStr, 
          wxFD_SAVE);

        if (dlg.ShowModal() != wxID_OK)
        {
          return;
        }

        editor->GetFile().FileSave(dlg.GetPath());
      }
      
      const wxBitmap bitmap = (editor->GetFileName().GetStat().IsOk() ? 
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(wxExGetIconID(editor->GetFileName())) : 
        wxNullBitmap);

      m_Editors->SetPageText(
        old_key,
        editor->GetFileName().GetFullPath(),
        editor->GetFileName().GetFullName(),
        bitmap);
          
      editor->PropertiesMessage();
    }
    break;

  case wxID_EXECUTE: m_Process->Execute(); break;
  case wxID_STOP: m_Process->Kill(); break;

  case ID_ALL_STC_CLOSE:
  case ID_ALL_STC_SAVE:
    m_Editors->ForEach(event.GetId());
    break;

  case ID_EDIT_MACRO: OpenFile(wxExViMacros::GetFileName()); break;
  case ID_EDIT_MACRO_PLAYBACK: if (editor != NULL) editor->GetVi().MacroPlayback(); break;
  case ID_EDIT_MACRO_START_RECORD: if (editor != NULL) editor->GetVi().MacroStartRecording(); break;
  case ID_EDIT_MACRO_STOP_RECORD: if (editor != NULL) editor->GetVi().MacroStopRecording(); break;
  
  case ID_OPTION_EDITOR:
    wxExSTC::ConfigDialog(this,
      _("Editor Options"),
      wxExSTC::STC_CONFIG_MODELESS | wxExSTC::STC_CONFIG_WITH_APPLY,
      event.GetId());
    break;

  case ID_OPTION_COMPARATOR: 
    {
      std::vector<wxExConfigItem> v;
      v.push_back(wxExConfigItem(_("Comparator"), CONFIG_FILEPICKERCTRL));
      wxExConfigDialog(
        this,
        v,
        _("Set Comparator")).ShowModal();
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

        if (m_Projects != NULL)
        {
          wxExForEach(m_Projects, ID_LIST_ALL_ITEMS, font);
        }
        
        wxExForEach(m_Lists, ID_LIST_ALL_ITEMS, font);
        
        if (m_History != NULL)
        {
          m_History->SetFont(font);
          m_History->ItemsUpdate();
        }
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
      vcs.GetDir(this);
      StatusText(vcs.GetName(), "PaneVCS");
    }
    break;
    
  case ID_PROCESS_SELECT: wxExProcess::ConfigDialog(this); break;

  case ID_PROJECT_CLOSE:
    if (project != NULL && m_Projects != NULL)
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
        OpenFile(project->GetFileName());
      }
    }
    break;
  case ID_PROJECT_SAVEAS:
    if (project != NULL && m_Projects != NULL)
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
      wxString::Format("split%06d", m_SplitId++),
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
    if (m_History == NULL)
    {
      AddPaneHistory();
      GetManager().Update();
    }
    else
    {
      TogglePane("HISTORY");
    }
    
#if wxUSE_STATUSBAR
    UpdateStatusBar(m_History);
#endif
    break;
  case ID_VIEW_OUTPUT: 
    TogglePane("OUTPUT");
    break;
    
  case ID_VIEW_PROJECTS: 
    if (m_Projects == NULL)
    {
      AddPaneProjects();
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
      event.Enable( m_Process->IsSelected() &&
                   !m_Process->IsRunning()); 
      break;
    case wxID_STOP: event.Enable(m_Process->IsRunning()); break;
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
      event.Check(m_History != NULL && GetManager().GetPane("HISTORY").IsShown());
      break;
    case ID_VIEW_OUTPUT:
      event.Check(GetManager().GetPane("OUTPUT").IsShown());
      break;
    case ID_VIEW_PROJECTS:
      event.Check(GetManager().GetPane("PROJECTS").IsShown());
      break;

    default:
    {
      wxExSTC* editor = GetSTC();
      wxExListViewFile* list = (wxExListViewFile*)GetListView();

      if (editor != NULL)
      {
        event.Enable(true);

        switch (event.GetId())
        {
        case wxID_JUMP_TO:
        case wxID_REPLACE:
        case wxID_SAVEAS:
        case ID_EDIT_FIND_NEXT:
        case ID_EDIT_FIND_PREVIOUS:
        case ID_EDIT_FOLD_ALL:
        case ID_EDIT_UNFOLD_ALL:
          event.Enable(editor->GetLength() > 0);
          break;

        case ID_EDIT_MACRO:
          event.Enable(
             editor->GetVi().GetIsActive() &&
            !editor->GetVi().MacroIsRecording() &&
             wxExViMacros::GetFileName().FileExists());
          break;
        case ID_EDIT_MACRO_PLAYBACK:
          event.Enable(editor->GetVi().MacroIsRecorded() && !editor->GetVi().MacroIsRecording());
          break;
        case ID_EDIT_MACRO_START_RECORD:
          event.Enable(
             editor->GetVi().GetIsActive() && 
            !editor->GetVi().MacroIsRecording());
          break;
        case ID_EDIT_MACRO_STOP_RECORD:
          event.Enable(editor->GetVi().MacroIsRecording());
          break;

        case ID_EDIT_TOGGLE_FOLD:
          event.Enable(
            editor->GetTextLength() > 0 &&
            editor->GetFoldLevel(editor->GetCurrentLine()) > wxSTC_FOLDLEVELBASE);
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

        if (
          event.GetId() > ID_TOOL_LOWEST &&
          event.GetId() < ID_TOOL_HIGHEST)
        {
          event.Enable(list->GetSelectedItemCount() > 0);
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

  wxWindow* page = m_Editors->SelectPage(key);
  
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
      
    const int index = m_Editors->GetPageIndexByKey(filename.GetFullPath());
    
    if (index != -1)
    {
      // Place new page before the one used for vcs.
      m_Editors->InsertPage(
        index,
        editor,
        key,
        filename.GetFullName() + " " + unique,
        true);
      }
    else
    {
      // Just add at the end.
      m_Editors->AddPage(
        editor,
        key,
        filename.GetFullName() + " " + unique,
        true);
    }
  }

  return true;
}

bool Frame::OpenFile(
  const wxString& filename,
  const wxString& text,
  long flags)
{
  const wxString key = filename;

  wxExSTCWithFrame* page = (wxExSTCWithFrame*)m_Editors->SelectPage(key);

  if (page == NULL)
  {
    wxExSTCWithFrame* editor = new wxExSTCWithFrame(
      m_Editors, 
      this,
      text,
      flags,
      filename);

    m_Editors->AddPage(
      editor,
      key,
      filename,
      true);
  }
  else
  {
    page->SetText(text);
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
  
  wxExNotebook* notebook = (flags & WIN_IS_PROJECT
    ? m_Projects : m_Editors);
    
  notebook->Freeze();

  wxWindow* page = notebook->SelectPage(filename.GetFullPath());

  if (flags & WIN_IS_PROJECT)
  {
    if (page == NULL)
    {
      wxExListViewFile* project = new wxExListViewFile(m_Projects,
        this,
        filename.GetFullPath(),
        wxID_ANY,
        wxExListViewWithFrame::LIST_MENU_DEFAULT);

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

    if (filename == wxExViMacros::GetFileName().GetFullPath())
    {
      wxExViMacros::SaveDocument();
    }
    
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
        m_DirCtrl->ExpandAndSelectPath(filename.GetFullPath());
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
    else if (!match.empty())
    {
      editor->FindNext(match);
    }
  }
  
  notebook->Thaw();

  return true;
}

void Frame::StatusBarDoubleClicked(const wxString& pane)
{
  if (pane == "PaneTheme")
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
  if (pane == "PaneLexer" || pane == "PaneTheme")
  {
    wxString match;
    
    if (pane == "PaneLexer")
    {
      wxExSTC* stc = GetSTC();
    
      if (stc != NULL && !stc->GetLexer().GetScintillaLexer().empty())
      {
        match = "name=\"" + stc->GetLexer().GetScintillaLexer();
      }
    }
    else
    {
      match = wxExLexers::Get()->GetTheme();
    }
    
    OpenFile(
      wxExLexers::Get()->GetFileName(),
      0,
      match);
  }
  else if (pane == "PaneVCS")
  {
    const wxString match = (GetStatusText("PaneVCS") != "Auto" ? 
      GetStatusText("PaneVCS"): wxString(wxEmptyString));
      
    OpenFile(
      wxExVCS::GetFileName(),
      0,
      match);
  }
  else
  {
    DecoratedFrame::StatusBarDoubleClickedRight(pane);
  }
}

void Frame::SyncCloseAll(wxWindowID id)
{
  DecoratedFrame::SyncCloseAll(id);
  
  if (m_IsClosing)
  {
    return;
  }
  
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
