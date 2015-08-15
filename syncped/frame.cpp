////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of class Frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/version.h>
#include <wx/extension/vimacros.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/util.h>
#include "frame.h"
#include "app.h"
#include "defs.h"

void About(wxFrame* frame)
{
  wxAboutDialogInfo info;
  info.SetIcon(frame->GetIcon());
  info.SetVersion(wxExGetVersionInfo().GetVersionOnlyString());

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
  info.SetWebSite("http://sourceforge.net/projects/syncped/");
    
  wxAboutBox(info);
}
    
BEGIN_EVENT_TABLE(Frame, DecoratedFrame)
  EVT_CHECKBOX(ID_CHECKBOX_DIRCTRL, Frame::OnCommand)
  EVT_CHECKBOX(ID_CHECKBOX_HISTORY, Frame::OnCommand)
  EVT_MENU(wxID_DELETE, Frame::OnCommand)
  EVT_MENU(wxID_EXECUTE, Frame::OnCommand)
  EVT_MENU(wxID_JUMP_TO, Frame::OnCommand)
  EVT_MENU(wxID_SELECTALL, Frame::OnCommand)
  EVT_MENU(wxID_STOP, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CLOSE, wxID_CLOSE_ALL, Frame::OnCommand)
  EVT_MENU_RANGE(ID_APPL_LOWEST, ID_APPL_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_ALL_LOWEST, ID_ALL_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, Frame::OnCommand)
  EVT_MENU_RANGE(ID_VCS_LOWEST, ID_VCS_HIGHEST, Frame::OnCommand)
  EVT_UPDATE_UI(ID_ALL_STC_CLOSE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_ALL_STC_SAVE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_CLOSE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_EXECUTE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_FIND, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_JUMP_TO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PRINT, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PREVIEW, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REPLACE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UNDO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REDO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_SAVE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_SAVEAS, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_STOP, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_CONTROL_CHAR, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_MENU, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_PLAYBACK, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_START_RECORD, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_STOP_RECORD, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_OPTION_VCS, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_PROJECT_SAVE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_FILE_MENU, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_PROJECT_MENU, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SORT_SYNC, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_EDIT_FIND_NEXT, ID_EDIT_FIND_PREVIOUS, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_EDIT_TOGGLE_FOLD, ID_EDIT_UNFOLD_ALL, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(
    ID_OPTION_LIST_SORT_ASCENDING, ID_OPTION_LIST_SORT_TOGGLE, Frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_PROJECT_OPENTEXT, ID_PROJECT_SAVEAS, Frame::OnUpdateUI)
END_EVENT_TABLE()

Frame::Frame(App* app)
  : DecoratedFrame()
  , m_IsClosing(false)
  , m_NewProjectNo(1)
  , m_SplitId(1)
  , m_History(NULL)
  , m_Process(new wxExProcess())
  , m_Projects(NULL)
  , m_asciiTable(NULL)
  , m_PaneFlag(
      wxAUI_NB_DEFAULT_STYLE |
      wxAUI_NB_CLOSE_ON_ALL_TABS |
      wxAUI_NB_CLOSE_BUTTON |
      wxAUI_NB_WINDOWLIST_BUTTON |
      wxAUI_NB_SCROLL_BUTTONS)
  , m_ProjectWildcard(_("Project Files") + " (*.prj)|*.prj")
  , m_Editors(new Notebook(
      this, 
      this, 
      (wxWindowID)NOTEBOOK_EDITORS, 
      wxDefaultPosition, 
      wxDefaultSize, 
      m_PaneFlag))
  , m_Lists(new wxExNotebook(
      this, 
      this, 
      (wxWindowID)NOTEBOOK_LISTS, 
      wxDefaultPosition, 
      wxDefaultSize, 
      m_PaneFlag))
  , m_DirCtrl(new wxExGenericDirCtrl(this, this))
  , m_CheckBoxDirCtrl(new wxCheckBox(
      GetToolBar(),
      ID_CHECKBOX_DIRCTRL,
      _("Explorer")))
  , m_CheckBoxHistory(new wxCheckBox(
      GetToolBar(),
      ID_CHECKBOX_HISTORY,
      _("History")))
  , m_App(app)
{
  wxExViMacros::LoadDocument();

  GetManager().AddPane(m_Editors, wxAuiPaneInfo()
    .CenterPane()
    .MaximizeButton(true)
    .Name("FILES")
    .Caption(_("Files")));

  if (wxConfigBase::Get()->ReadBool("ShowProjects", false))
  {
    AddPaneProjects();
  }

  GetManager().AddPane(m_DirCtrl, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .CloseButton(false)
    .Name("DIRCTRL")
    .MinSize(200, 150)
    .Caption(_("Explorer")));

  GetManager().AddPane(m_Lists, wxAuiPaneInfo()
    .Bottom()
    .MaximizeButton(true)
    .MinSize(250, 100)
    .Name("OUTPUT")
    .Row(0)
    .Caption(_("Output")));

  const wxString perspective = wxConfigBase::Get()->Read("Perspective");

  if (perspective.empty())
  {
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
  
  if (m_App->GetFiles().empty())
  {
    long count = 0;
    m_App->Reset();
      
    if (wxConfigBase::Get()->Read("OpenFiles", &count))
    {
      if (count > 0)
      {
        wxExOpenFiles(this, 
          wxExToVectorString(GetFileHistory(), count).Get());
      }
    }
      
    if (GetManager().GetPane("PROJECTS").IsShown() && m_Projects != NULL)
    {
      if (!GetRecentProject().empty())
      {
        OpenFile(
          wxExFileName(GetRecentProject()),
          0,
          wxEmptyString,
          0,
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
    wxExOpenFiles(this, m_App->GetFiles(), 0, wxDIR_FILES); // only files in this dir
    m_App->Reset();
  }
  
  StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
  
  // End with update, so all changes in the manager are handled.
  GetManager().Update();
  
  if (m_Editors->GetPageCount() > 0)
  {
    m_Editors->GetPage(m_Editors->GetPageCount() - 1)->SetFocus();
  }

  GetToolBar()->AddControl(m_CheckBoxDirCtrl);
  m_CheckBoxDirCtrl->SetToolTip(_("Explorer"));
  GetToolBar()->AddControl(m_CheckBoxHistory);
  m_CheckBoxHistory->SetToolTip(_("History"));
  GetToolBar()->Realize();
  m_CheckBoxDirCtrl->SetValue(GetManager().GetPane("DIRCTRL").IsShown());
  m_CheckBoxHistory->SetValue(
    wxConfigBase::Get()->ReadBool("ShowHistory", false));
    
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_IsClosing = true; 
    long count = 0;
    for (size_t i = 0; i < m_Editors->GetPageCount(); i++)
    {
      wxExSTC* stc = wxDynamicCast(m_Editors->GetPage(i), wxExSTC);
      
      if (stc->GetFileName().FileExists())
      {
        count++;
      }
    }
    if (event.CanVeto())
    {
      if (
         m_Process->IsRunning() || 
        !m_Editors->ForEach(ID_ALL_STC_CLOSE) || 
        !wxExForEach(m_Projects, ID_LIST_ALL_CLOSE))
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
    event.Skip();});
    
  Bind(wxEVT_AUINOTEBOOK_BG_DCLICK, [=](wxAuiNotebookEvent& event) {
    FileHistoryPopupMenu();}, NOTEBOOK_EDITORS);
    
  Bind(wxEVT_AUINOTEBOOK_BG_DCLICK, [=] (wxAuiNotebookEvent& event) {
    ProjectHistoryPopupMenu();}, NOTEBOOK_PROJECTS);
    
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(
      m_asciiTable != NULL && 
      GetManager().GetPane("ASCIITABLE").IsShown());}, ID_VIEW_ASCII_TABLE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("DIRCTRL").IsShown());}, ID_VIEW_DIRCTRL);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("FILES").IsShown());}, ID_VIEW_FILES);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(m_History != NULL && GetManager().GetPane("HISTORY").IsShown());}, ID_VIEW_HISTORY);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("OUTPUT").IsShown());}, ID_VIEW_OUTPUT);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(m_Projects != NULL && GetManager().GetPane("PROJECTS").IsShown());}, ID_VIEW_PROJECTS);
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

void Frame::AddAsciiTable()
{
  m_asciiTable = new wxExSTC(this);

  GetManager().AddPane(m_asciiTable, wxAuiPaneInfo()
    .Left()
    .Name("ASCIITABLE")
    .MinSize(500, 150)
    .Caption(_("Ascii Table")));
        
  GetManager().Update();

  // Do not show an edge, eol or whitespace for ascii table.
  m_asciiTable->SetEdgeMode(wxSTC_EDGE_NONE);
  m_asciiTable->SetViewEOL(false);
  m_asciiTable->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  // And override tab width.
  m_asciiTable->SetTabWidth(5);

  for (int i = 1; i <= 255; i++)
  {
    switch (i)
    {
      case  9: m_asciiTable->AddText(wxString::Format("%3d\tTAB", i)); break;
      case 10: m_asciiTable->AddText(wxString::Format("%3d\tLF", i)); break;
      case 13: m_asciiTable->AddText(wxString::Format("%3d\tCR", i)); break;
        
      default:
        m_asciiTable->AddText(wxString::Format("%3d\t%c", i, (wxUniChar)i));
    }
    
    m_asciiTable->AddText((i % 5 == 0) ? m_asciiTable->GetEOL(): "\t");
  }

  m_asciiTable->EmptyUndoBuffer();
  m_asciiTable->SetSavePoint();
  m_asciiTable->SetReadOnly(true);
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
  wxASSERT(m_History == NULL);
  
  m_History = new wxExListViewWithFrame(this, this,
    wxExListViewFileName::LIST_HISTORY,
    wxExListViewWithFrame::LIST_MENU_DEFAULT);
        
  GetManager().AddPane(m_History, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .CloseButton(false)
    .MinSize(150, 150)
    .Name("HISTORY")
    .Caption(_("History")));
}
       
void Frame::AddPaneProjects()
{
  if (m_Projects == NULL)
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
      .MinSize(150, 150)
      .Caption(_("Projects")));
  }
}

bool Frame::DialogProjectOpen()
{
  wxFileDialog dlg(this,
    _("Select Projects"),
     (!GetRecentProject().empty() ? 
         wxPathOnly(GetRecentProject()):
#ifdef wxExUSE_PORTABLE
         wxPathOnly(wxStandardPaths::Get().GetExecutablePath())),
#else
         wxStandardPaths::Get().GetUserDataDir()),
#endif
    wxEmptyString,
    m_ProjectWildcard,
    wxFD_OPEN | wxFD_MULTIPLE);

  if (dlg.ShowModal() == wxID_CANCEL) return false;

  wxExOpenFiles(this, wxExToVectorString(dlg).Get(), WIN_IS_PROJECT);

  return true;
}

wxExSTC* Frame::ExecExCommand(int command)
{
  if (m_Editors->GetPageCount() == 0)
  {
    return NULL;
  }

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
  if (m_Projects == NULL)
  {
    return NULL;
  }

  if (!m_Projects->IsShown() || 
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

void Frame::NewFile(const wxString& name)
{
  if (wxConfigBase::Get()->ReadBool("HexMode", false))
  {
    // In hex mode we cannot edit the file.
    return;
  }
 
  wxWindow* page = new wxExSTC(
    m_Editors, 
    wxEmptyString,
    wxExSTC::STC_WIN_DEFAULT,
    wxEmptyString,
    0xFFFF);

  ((wxExSTC*)page)->GetFile().FileNew(name);

  // This file does yet exist, so do not give it a bitmap.
  m_Editors->AddPage(
    page,
    name,
    name,
    true);

  GetManager().GetPane("FILES").Show();
  GetManager().Update();
}

void Frame::NewProject()
{
  AddPaneProjects();
  
  const wxString text = wxString::Format("%s%d", _("project"), m_NewProjectNo++);
  const wxFileName fn(
     (!GetRecentProject().empty() ? 
         wxPathOnly(GetRecentProject()):
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())),
#else
      wxStandardPaths::Get().GetUserDataDir()),
#endif
    text + ".prj");

  wxWindow* page = new wxExListViewFile(m_Projects,
    this,
    fn.GetFullPath(),
    wxID_ANY,
    wxExListViewWithFrame::LIST_MENU_DEFAULT);

  ((wxExListViewFile*)page)->FileNew(fn.GetFullPath());

  // This file does yet exist, so do not give it a bitmap.
  m_Projects->AddPage(
    page,
    fn.GetFullPath(),
    text,
    true);
    
  SetRecentProject(fn.GetFullPath());

  GetManager().GetPane("PROJECTS").Show();
  GetManager().Update();
}

void Frame::OnCommand(wxCommandEvent& event)
{
  wxExSTC* editor = GetSTC();
  wxExListViewFile* project = GetProject();

  switch (event.GetId())
  {
  // edit commands
  // Do not change the wxID* in wxID_LOWEST and wdID_HIGHEST,
  // as wxID_ABOUT etc. is used here and not in the editor.
  // That causes appl to hang.
  case wxID_UNDO:
  case wxID_REDO:
  case wxID_DELETE:
  case wxID_SELECTALL:
  case wxID_JUMP_TO:
  case wxID_CUT:
  case wxID_COPY:
  case wxID_PASTE:
  case wxID_CLEAR:
    if (editor != NULL)
    {
      wxPostEvent(editor, event);
    }
  break;

  case wxID_ABOUT: About(this); break;
  
  case wxID_CLOSE:
    if (editor != NULL)
    {
      if (!AllowClose(
        m_Editors->GetId(),
        editor))
      {
        return;
      }
        
      m_Editors->DeletePage(m_Editors->GetKeyByPage(editor));
    }
    break;
    
  case wxID_EXIT: Close(true); break;
  
  case wxID_HELP:
    wxLaunchDefaultBrowser(
      "http://antonvw.github.io/syncped/v" + 
      wxExGetVersionInfo().GetVersionOnlyString() + 
      "/syncped.htm");
    break;
    
  case wxID_NEW: 
      if (!event.GetString().empty())
      {
        NewFile(event.GetString()); 
      }
      else
      {
        static wxString text;
        wxTextEntryDialog dlg(this, _("Input") + ":", _("File Name"), text);
        
        wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
        validator.SetCharExcludes("/\\?%*:|\"<>");
        dlg.SetTextValidator(validator);
        
        if (dlg.ShowModal() == wxID_CANCEL)
        {
          return;
        }
        
        text = dlg.GetValue();
        
        NewFile(text); 
      }
    break;
  
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
      if (!editor->GetFile().FileSave())
      {
        return;
      }

      SetRecentFile(editor->GetFileName().GetFullPath());
      
      if (editor->GetFileName() == wxExLexers::Get()->GetFileName())
      {
        wxExLexers::Get()->LoadDocument();
        m_Editors->ForEach(ID_ALL_STC_SET_LEXER);

        // As the lexer might have changed, update status bar field as well.
#if wxUSE_STATUSBAR
        UpdateStatusBar(editor, "PaneLexer");
#endif
      }
      else if (editor->GetFileName() == wxExVCS::GetFileName())
      {
        wxExVCS::LoadDocument();
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
      if (!event.GetString().empty())
      {
        if (!editor->GetFile().FileSave(event.GetString()))
        {
          return;
        }
      }
      else
      {
        wxExFileDialog dlg(
          this, 
          &editor->GetFile(), 
          wxGetStockLabel(wxID_SAVEAS, wxSTOCK_NOFLAGS), 
          wxFileSelectorDefaultWildcardStr, 
          wxFD_SAVE);

        if (dlg.ShowModal() != wxID_OK)
        {
          return;
        }

        if (!editor->GetFile().FileSave(dlg.GetPath()))
        {
          return;
        }
      }
      
      const wxBitmap bitmap = (editor->GetFileName().GetStat().IsOk() ? 
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(wxExGetIconID(editor->GetFileName())) : 
        wxNullBitmap);

      m_Editors->SetPageText(
        m_Editors->GetKeyByPage(editor),
        editor->GetFileName().GetFullPath(),
        editor->GetFileName().GetFullName(),
        bitmap);
          
      editor->PropertiesMessage();
      
      SetRecentFile(editor->GetFileName().GetFullPath());
    }
    break;

  case wxID_EXECUTE: m_Process->Execute(); break;
  case wxID_STOP: m_Process->Kill(); break;

  case ID_ALL_STC_CLOSE:
  case ID_ALL_STC_CLOSE_OTHERS:
  case ID_ALL_STC_SAVE:
    m_Editors->ForEach(event.GetId());
    break;

  case ID_EDIT_MACRO: OpenFile(wxExViMacros::GetFileName()); break;
  case ID_EDIT_MACRO_PLAYBACK: if (editor != NULL) editor->GetVi().MacroPlayback(); break;
  case ID_EDIT_MACRO_START_RECORD: if (editor != NULL) editor->GetVi().MacroStartRecording(); break;
  case ID_EDIT_MACRO_STOP_RECORD: if (editor != NULL) editor->GetVi().GetMacros().StopRecording(); break;
  
  case ID_OPTION_COMPARATOR: 
      wxExConfigDialog(
        this,
        std::vector<wxExConfigItem>
          {wxExConfigItem(_("Comparator"), CONFIG_FILEPICKERCTRL)},
        _("Set Comparator")).ShowModal();
    break;

  case ID_OPTION_LIST_FONT:
      if (wxExConfigDialog(
        this,
        std::vector<wxExConfigItem>
          {wxExConfigItem(_("List Font"), CONFIG_FONTPICKERCTRL)},
        _("Set List Font")).ShowModal() == wxID_OK)
      {
        const wxFont font(
          wxConfigBase::Get()->ReadObject(_("List Font"), 
            wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));

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
    break;

  case ID_OPTION_LIST_READONLY_COLOUR:
      if (!wxConfigBase::Get()->Exists(_("List Colour")))
      {
        wxConfigBase::Get()->Write(_("List Colour"), wxColour("RED"));
      }

      // text also used in menu
      wxExConfigDialog(
        this,
        std::vector<wxExConfigItem>
          {wxExConfigItem(_("List Colour"), CONFIG_COLOUR)},
        _("Set List Read Only Colour")).ShowModal();
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
      vcs.SetEntryFromBase(this);
      
      m_StatusBar->ShowField(
        "PaneVCS", 
        vcs.Use());
        
      StatusText(vcs.GetName(), "PaneVCS");
    }
    break;
    
  case ID_PROCESS_SELECT: 
    if (wxExProcess::ConfigDialog(this) == wxID_OK)
    {
      m_Process->Execute();
    }
    break;

  case ID_PROJECT_CLOSE:
    if (project != NULL && m_Projects != NULL)
    {
      m_Projects->DeletePage(project->GetFileName().GetFullPath());
    }
    break;
  case ID_PROJECT_NEW: NewProject(); break;
  case ID_PROJECT_OPEN:
    DialogProjectOpen();
    break;
  case ID_PROJECT_OPENTEXT:
    if (project != NULL)
    {
      if (wxExFileDialog(
        this, project).ShowModalIfChanged() != wxID_CANCEL)
      {
        OpenFile(project->GetFileName());
      }
    }
    break;
  case ID_PROJECT_SAVEAS:
    if (project != NULL && m_Projects != NULL)
    {
      wxExFileDialog dlg(
        this, project, 
        _("Project Save As"), 
        m_ProjectWildcard, 
        wxFD_SAVE);

      if (dlg.ShowModal() == wxID_OK)
      {
        project->FileSave(dlg.GetPath());

        m_Projects->SetPageText(
          m_Projects->GetKeyByPage(project),
          project->GetFileName().GetFullPath(),
          project->GetFileName().GetName());
      }
    }
    break;

  case ID_SORT_SYNC: 
    wxConfigBase::Get()->Write("List/SortSync", 
      !wxConfigBase::Get()->ReadBool("List/SortSync", true)); 
    break;

  case ID_REARRANGE_HORIZONTALLY: m_Editors->Rearrange(wxTOP); break;
  case ID_REARRANGE_VERTICALLY: m_Editors->Rearrange(wxLEFT); break;
    
  case ID_SPLIT:
  case ID_SPLIT_HORIZONTALLY:
  case ID_SPLIT_VERTICALLY:
    {
      wxExSTC* stc = new wxExSTC(*editor);
      editor->Sync(false);
      stc->Sync(false);

      wxBitmap bitmap(wxNullBitmap);
      
      if (stc->GetFileName().FileExists())
      {
        bitmap = wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wxExGetIconID(stc->GetFileName()));
      }
      
      // key should be unique
      const wxString key(wxString::Format("split%06d", m_SplitId++));
      
      // Place new page before page for editor.
      m_Editors->InsertPage(
        m_Editors->GetPageIndex(editor),
        stc,
        key,
        stc->GetFileName().GetFullName(),
        true,
        bitmap);

      stc->SetDocPointer(editor->GetDocPointer());
      
      if (event.GetId() == ID_SPLIT_HORIZONTALLY)
      {
        m_Editors->Split(key, wxBOTTOM);
        m_Editors->SetSelection(editor->GetFileName().GetFullPath());
      }
      else if (event.GetId() == ID_SPLIT_VERTICALLY)
      {
        m_Editors->Split(key, wxRIGHT);
        m_Editors->SetSelection(editor->GetFileName().GetFullPath());
      }
    }
    break;
    
  case ID_VIEW_ASCII_TABLE: 
    if (m_asciiTable == NULL)
    {
      AddAsciiTable();
    }
    else
    {
      TogglePane("ASCIITABLE"); 
    }
    break;
    
  case ID_CHECKBOX_DIRCTRL: 
  case ID_VIEW_DIRCTRL: 
    TogglePane("DIRCTRL"); 
    m_CheckBoxDirCtrl->SetValue(GetManager().GetPane("DIRCTRL").IsShown());
    GetToolBar()->Realize();
    
    if (GetManager().GetPane("DIRCTRL").IsShown() &&
        GetManager().GetPane("FILES").IsShown())
    {
      wxExSTC* editor = GetSTC();
      
      if (editor != NULL)
      {
        m_DirCtrl->ExpandAndSelectPath(
          editor->GetFileName().GetFullPath());
      }
    }
    break;
  
  case ID_VIEW_FILES: 
    TogglePane("FILES"); 
    
    if (!GetManager().GetPane("FILES").IsShown())
    {
      if (GetManager().GetPane("PROJECTS").IsShown())
      {
        GetManager().GetPane("PROJECTS").Maximize();
        GetManager().Update();
      }
    }
    break;
    
  case ID_VIEW_HISTORY: 
  case ID_CHECKBOX_HISTORY: 
    if (m_History == NULL)
    {
      AddPaneHistory();
      GetManager().Update();
    }
    else
    {
      TogglePane("HISTORY");
    }
  
    m_CheckBoxHistory->SetValue(GetManager().GetPane("HISTORY").IsShown());
    GetToolBar()->Realize();
    
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
      NewProject();
    }
    else
    {
      TogglePane("PROJECTS");
    }
    break;
    
  default: 
    if (event.GetId() > ID_VCS_LOWEST && event.GetId() < ID_VCS_HIGHEST)
    {
      wxExVCS(std::vector< wxString >(), event.GetId() - ID_VCS_LOWEST - 1).Request(this);
    }
    else if (event.GetId() > ID_EDIT_STC_LOWEST && event.GetId() < ID_EDIT_STC_HIGHEST)
    {
      if (editor != NULL)
      {
        wxPostEvent(editor, event);
      }
    }
    else
      wxFAIL;
    break;
  }
}

void Frame::OnCommandConfigDialog(
  wxWindowID dialogid,
  int commandid)
{
  if (dialogid == wxID_PREFERENCES)
  {
    if (commandid != wxID_CANCEL)
    {
      m_Editors->ForEach(ID_ALL_STC_CONFIG_GET);
      
      if (m_Process->GetSTC() != NULL)
      {
        m_Process->GetSTC()->ConfigGet();
      }
      
      m_StatusBar->ShowField(
        "PaneMacro", 
        wxConfigBase::Get()->ReadBool(_("vi mode"), true));
    }
  }
  else
  {
    DecoratedFrame::OnCommandConfigDialog(dialogid, commandid);
  }
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

    case wxID_CLOSE:
    case wxID_SAVEAS:
      event.Enable(m_Editors->IsShown() && m_Editors->GetPageCount() > 0);
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

    default:
    {
      wxExSTC* editor = GetSTC();
      wxExListViewFile* list = (wxExListViewFile*)GetListView();

      if (editor != NULL)
      {
        event.Enable(true);

        switch (event.GetId())
        {
        case wxID_FIND:
        case wxID_JUMP_TO:
        case wxID_REPLACE:
        case ID_EDIT_FIND_NEXT:
        case ID_EDIT_FIND_PREVIOUS:
        case ID_EDIT_FOLD_ALL:
        case ID_EDIT_UNFOLD_ALL:
          event.Enable(editor->GetLength() > 0);
          break;
        case ID_EDIT_MACRO:
          event.Enable(
             editor->GetVi().GetIsActive() &&
            !editor->GetVi().GetMacros().IsRecording() &&
             wxExViMacros::GetFileName().FileExists());
          break;
        case ID_EDIT_MACRO_MENU:
          event.Enable(
             editor->GetVi().GetIsActive());
          break;
        case ID_EDIT_MACRO_PLAYBACK:
          event.Enable(
             editor->GetVi().GetIsActive() &&
             editor->GetVi().GetMacros().GetCount() > 0 &&
            !editor->GetVi().GetMacros().IsRecording());
          break;
        case ID_EDIT_MACRO_START_RECORD:
          event.Enable(
             editor->GetVi().GetIsActive() && 
            !editor->GetVi().GetMacros().IsRecording());
          break;
        case ID_EDIT_MACRO_STOP_RECORD:
          event.Enable(editor->GetVi().GetMacros().IsRecording());
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
      else if (list != NULL && list->IsShown())
      {
        event.Enable(false);

        if (
          event.GetId() > ID_TOOL_LOWEST &&
          event.GetId() < ID_TOOL_HIGHEST)
        {
          event.Enable(list->GetSelectedItemCount() > 0);
        }
        else if (event.GetId() == wxID_FIND)
        {
          event.Enable(list->GetItemCount() > 0);
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
    
  const wxString key = filename.GetFullPath() + " " + unique;

  wxWindow* page = m_Editors->SetSelection(key);
  
  if (page == NULL)
  {
    wxExSTC* editor = new wxExSTC(
      m_Editors, 
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
  wxExSTC* page = (wxExSTC*)m_Editors->SetSelection(filename);

  if (page == NULL)
  {
    m_Editors->AddPage(new wxExSTC(m_Editors, text, flags, filename), filename, filename, true);
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
  int col_number,
  long flags)
{
  if ((flags & WIN_IS_PROJECT) && m_Projects == NULL)
  {
    AddPaneProjects();
    GetManager().Update();
  }
  
  wxExNotebook* notebook = ((flags & WIN_IS_PROJECT)
    ? m_Projects : m_Editors);
    
  wxASSERT(notebook != NULL);
  
  wxWindow* page = notebook->SetSelection(filename.GetFullPath());

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
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wxExGetIconID(filename)));
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

    if (filename == wxExViMacros::GetFileName().GetFullPath())
    {
      wxExViMacros::SaveDocument();
    }
    
    wxExSTC* editor = (wxExSTC*)page;

    if (page == NULL)
    {
      if (wxConfigBase::Get()->ReadBool("HexMode", false))
        flags |= wxExSTC::STC_WIN_HEX;

      editor = new wxExSTC(m_Editors,
        filename,
        line_number,
        match,
        col_number,
        flags,
        0xFFFF);
        
      const wxString key(filename.GetFullPath());

      notebook->AddPage(
        editor,
        key,
        filename.GetFullName(),
        true,
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wxExGetIconID(filename)));
          
      if (notebook->GetPageCount() >= 2 && m_App->GetSplit() != -1)
      {
        notebook->Split(key, m_App->GetSplit());
      }

      if (!m_App->GetCommand().empty() && editor->GetVi().GetIsActive())
      {
        if (!editor->GetVi().Command(m_App->GetCommand().ToStdString()))
        {
          wxLogError("Command error: " + m_App->GetCommand());
        }
      }
      
      if (GetManager().GetPane("DIRCTRL").IsShown())
      {
        m_DirCtrl->ExpandAndSelectPath(key);
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
  
  return true;
}

void Frame::PrintEx(wxExEx* ex, const wxString& text)
{
  wxExSTC* page = (wxExSTC*)m_Editors->SetSelection("Print");

  if (page == NULL)
  {
    m_Editors->AddPage(new wxExSTC(m_Editors, text, (long)wxExSTC::STC_WIN_DEFAULT, "Print"), "Print", "Print", true);
  }
  else
  {
    page->AppendText(text);
    page->DocumentEnd();
    page->SetSavePoint();
  }
}
  
void Frame::StatusBarClicked(const wxString& pane)
{
  if (pane == "PaneTheme")
  {
    if (wxExLexers::Get()->ShowThemeDialog(m_Editors))
    {
      m_Editors->ForEach(ID_ALL_STC_SET_LEXER_THEME);

      m_StatusBar->ShowField(
        "PaneLexer", 
        wxExLexers::Get()->GetThemeOk());
        
      StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
    }
  }
  else if (pane == "PaneMacro")
  {
    wxExSTC* editor = GetSTC();
    if (editor != NULL) editor->GetVi().MacroPlayback();
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
    DecoratedFrame::StatusBarClicked(pane);
  }
}

void Frame::StatusBarClickedRight(const wxString& pane)
{
  if (pane == "PaneLexer" || pane == "PaneTheme")
  {
    wxString match;
    
    if (pane == "PaneLexer")
    {
      wxExSTC* stc = GetSTC();
    
      if (stc != NULL)
      {
        if (
          !stc->GetLexer().GetScintillaLexer().empty() && 
           stc->GetLexer().GetScintillaLexer() == stc->GetLexer().GetDisplayLexer())
        {
          match = "lexer name=\"" + stc->GetLexer().GetScintillaLexer() + "\"";
        }
        else if (!stc->GetLexer().GetDisplayLexer().empty())
        {
          match = "display=\"" + stc->GetLexer().GetDisplayLexer() + "\"";
        }
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
  else if (pane == "PaneMacro")
  {
    wxExSTC* stc = GetSTC();
    
    if ((stc != NULL && !stc->GetVi().GetIsActive()) || 
        !wxExViMacros::GetFileName().FileExists())
    {
      return;
    }
    
    const wxString  macro(GetStatusText(pane));
      
    if (stc->GetVi().GetMacros().IsRecordedMacro(macro))
    {
      OpenFile(wxExViMacros::GetFileName(),
        0,
        "macro name=\"" + macro + "\"");
    }
    else
    {
      OpenFile(wxExViMacros::GetFileName(),
        0,
        "variable name=\"" + macro + "\"");
    }
  }
  else if (pane == "PaneVCS")
  {
    const wxString match = (GetStatusText(pane) != "Auto" ? 
      GetStatusText(pane): wxString(wxEmptyString));
      
    OpenFile(
      wxExVCS::GetFileName(),
      0,
      match);
  }
  else
  {
    DecoratedFrame::StatusBarClickedRight(pane);
  }
}

void Frame::SyncAll()
{
  m_Editors->ForEach(ID_ALL_STC_SYNC);
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
    SetTitle(wxTheApp->GetAppDisplayName());
    StatusText(wxEmptyString, wxEmptyString);
    StatusText(wxEmptyString, "PaneFileType");
    StatusText(wxEmptyString, "PaneInfo");
    StatusText(wxEmptyString, "PaneLexer");
    
    if (GetManager().GetPane("PROJECTS").IsShown() && m_Projects != NULL)
    {
      GetManager().GetPane("PROJECTS").Maximize();
      GetManager().Update();
    }
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

Notebook::Notebook(wxWindow* parent,
  wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExNotebook(parent, frame, id, pos, size, style)
{
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      wxPostEvent(GetCurrentPage(), event);
    }, ID_EDIT_VCS_LOWEST, ID_EDIT_VCS_HIGHEST);
  
  Bind(wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, [=](wxAuiNotebookEvent& event) {
    wxExMenu menu;
    
    wxExMenu* split = new wxExMenu;
    split->Append(ID_SPLIT_VERTICALLY, _("Split Vertically"));
    split->Append(ID_SPLIT_HORIZONTALLY, _("Split Horizontally"));
    split->AppendSeparator();
    split->Append(ID_SPLIT, _("Split"));
    
    if (GetPageCount() > 1)
    {
      split->AppendSeparator();
      split->Append(ID_REARRANGE_VERTICALLY, _("Rearrange Vertically"));
      split->Append(ID_REARRANGE_HORIZONTALLY, _("Rearrange Horizontally"));
    }

    menu.AppendSubMenu(split, _("Split"), wxEmptyString, ID_SPLIT_MENU);
    menu.AppendSeparator();
    menu.Append(wxID_CLOSE);
    menu.Append(ID_ALL_STC_CLOSE, _("Close A&ll"));
    
    if (GetPageCount() > 2)
    {
      menu.Append(ID_ALL_STC_CLOSE_OTHERS, _("Close Others"));
    }

    wxExSTC* stc = wxDynamicCast(GetCurrentPage(), wxExSTC);
    
    if (stc->GetFile().GetFileName().FileExists() && 
        wxExVCS::DirExists(stc->GetFile().GetFileName()))
    {
      menu.AppendSeparator();
      menu.AppendVCS(stc->GetFile().GetFileName());
    }
    
    PopupMenu(&menu);});
}
