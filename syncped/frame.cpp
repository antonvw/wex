////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of class Frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/config.h>
#include <wx/imaglist.h>
#include <wx/extension/ctags.h>
#include <wx/extension/debug.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/menu.h>
#include <wx/extension/menus.h>
#include <wx/extension/otl.h>
#include <wx/extension/printing.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/tostring.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/version.h>
#include <wx/extension/vi-macros.h>
#include <wx/extension/vi-macros-mode.h>
#include <wx/extension/report/listviewfile.h>
#include "frame.h"
#include "app.h"
#include "defs.h"

const int ID_EDIT_PANE_INFO_TOGGLE = wxWindow::NewControlId();

class EditorsNotebook : public wxExNotebook
{
public:
  EditorsNotebook(const wxExWindowData& data); 

  /// Returns true if notebook is splitted.
  bool IsSplit() const {return m_Split;};

  /// Reset members.
  void Reset() {m_Split = false;};
protected:
  bool m_Split {false};
};

BEGIN_EVENT_TABLE(Frame, DecoratedFrame)
  EVT_MENU(wxID_DELETE, Frame::OnCommand)
  EVT_MENU(wxID_EXECUTE, Frame::OnCommand)
  EVT_MENU(wxID_JUMP_TO, Frame::OnCommand)
  EVT_MENU(wxID_SELECTALL, Frame::OnCommand)
  EVT_MENU(wxID_STOP, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CLOSE, wxID_CLOSE_ALL, Frame::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_MACRO_PLAYBACK, ID_EDIT_MACRO_STOP_RECORD, Frame::OnCommand)
  EVT_MENU_RANGE(ID_SPLIT, ID_SPLIT_VERTICALLY, Frame::OnCommand)
  EVT_UPDATE_UI(ID_ALL_CLOSE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_ALL_SAVE, Frame::OnUpdateUI)
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
  EVT_UPDATE_UI_RANGE(ID_PROJECT_OPENTEXT, ID_PROJECT_SAVEAS, Frame::OnUpdateUI)
END_EVENT_TABLE()

Frame::Frame(App* app)
  : DecoratedFrame(app)
  , m_Process(new wxExProcess())
  , m_Editors(new EditorsNotebook(wxExWindowData().Id(ID_NOTEBOOK_EDITORS).Style(m_PaneFlag)))
  , m_Lists(new wxExNotebook(
      wxExWindowData().Id(ID_NOTEBOOK_LISTS).Style(m_PaneFlag)))
  , m_DirCtrl(new wxExGenericDirCtrl(this))
  , m_CheckBoxDirCtrl(new wxCheckBox(
      GetToolBar(),
      ID_VIEW_DIRCTRL,
      _("Explorer")))
  , m_CheckBoxHistory(new wxCheckBox(
      GetToolBar(),
      ID_VIEW_HISTORY,
      _("History")))
{
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

  GetManager().AddPane(m_Process->GetShell(), wxAuiPaneInfo()
    .Bottom()
    .Name("PROCESS")
    .MinSize(250, 100)
    .Caption(_("Process")));
        
  if (const wxString perspective(wxConfigBase::Get()->Read("Perspective")); 
    perspective.empty())
  {
    GetManager().GetPane("DIRCTRL").Hide();
    GetManager().GetPane("HISTORY").Hide();
    GetManager().GetPane("LOG").Hide();
    GetManager().GetPane("PROCESS").Hide();
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
  
  if (!m_App->GetTag().empty())
  {
    wxExCTags(this).Find(m_App->GetTag());
  }
  else if (m_App->GetFiles().empty())
  {
    if (long count = 0; 
      wxConfigBase::Get()->Read("OpenFiles", &count) && count > 0)
    {
      wxExOpenFiles(this, GetFileHistory().GetHistoryFiles(count), 
        m_App->GetData());
    }
      
    if (GetManager().GetPane("PROJECTS").IsShown() && m_Projects != nullptr)
    {
      if (!GetProjectHistory().GetHistoryFile().Path().empty())
      {
        OpenFile(
          wxExPath(GetProjectHistory().GetHistoryFile()),
          wxExSTCData().Flags(STC_WIN_IS_PROJECT));
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
    wxExOpenFiles(this, m_App->GetFiles(), m_App->GetData(), DIR_FILES);
  }
  
  StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
  
  // End with update, so all changes in the manager are handled.
  GetManager().Update();
  
  if (m_Editors->GetPageCount() > 0)
  {
    m_Editors->GetPage(m_Editors->GetPageCount() - 1)->SetFocus();
  }

  GetToolBar()->AddControls(false); // no realize yet
  GetToolBar()->AddControl(m_CheckBoxDirCtrl);
  m_CheckBoxDirCtrl->SetToolTip(_("Explorer"));
  GetToolBar()->AddControl(m_CheckBoxHistory);
  m_CheckBoxHistory->SetToolTip(_("History"));
  GetToolBar()->Realize();
  
  GetOptionsToolBar()->AddControls();
  
  m_CheckBoxDirCtrl->SetValue(GetManager().GetPane("DIRCTRL").IsShown());
  m_CheckBoxHistory->SetValue(
    wxConfigBase::Get()->ReadBool("ShowHistory", false));
    
  Bind(wxEVT_AUINOTEBOOK_BG_DCLICK, [=](wxAuiNotebookEvent& event) {
    GetFileHistory().PopupMenu(this, ID_CLEAR_FILES);}, ID_NOTEBOOK_EDITORS);
    
  Bind(wxEVT_AUINOTEBOOK_BG_DCLICK, [=] (wxAuiNotebookEvent& event) {
    GetProjectHistory().PopupMenu(this, ID_CLEAR_PROJECTS);}, ID_NOTEBOOK_PROJECTS);
    
  Bind(wxEVT_CHECKBOX, [=] (wxCommandEvent& event) {
    TogglePane("DIRCTRL"); 
    m_CheckBoxDirCtrl->SetValue(GetManager().GetPane("DIRCTRL").IsShown());
    GetToolBar()->Realize();
    if (GetManager().GetPane("DIRCTRL").IsShown() &&
        GetManager().GetPane("FILES").IsShown())
    {
      if (auto* editor = GetSTC(); editor != nullptr)
      {
        m_DirCtrl->ExpandAndSelectPath(editor->GetFileName());
      }
    }}, ID_VIEW_DIRCTRL);

  Bind(wxEVT_CHECKBOX, [=] (wxCommandEvent& event) {
    if (m_History == nullptr)
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
    UpdateStatusBar(m_History);
    }, ID_VIEW_HISTORY);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    GetDebug()->Execute(event.GetId() - ID_EDIT_DEBUG_FIRST);}, 
    ID_EDIT_DEBUG_FIRST, ID_EDIT_DEBUG_LAST);
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    long count = 0;
    for (size_t i = 0; i < m_Editors->GetPageCount(); i++)
    {
      if (auto* stc = wxDynamicCast(m_Editors->GetPage(i), wxExSTC);
        stc->GetFileName().FileExists())
      {
        count++;
      }
    }
    if (event.CanVeto())
    {
      if ((m_Process->IsRunning() && m_Process->GetExecuteCommand() != "gdb") || 
        !m_Editors->ForEach<wxExSTC>(ID_ALL_CLOSE) || 
        (m_Projects != nullptr && !m_Projects->ForEach<wxExListViewFile>(ID_ALL_CLOSE)))
      {
        event.Veto();
        if (m_Process->IsRunning())
        {
          wxLogStatus(_("Process is running"));
        }
        return;
      }
    }
    wxExViMacros::SaveDocument();
    wxConfigBase::Get()->Write("Perspective", GetManager().SavePerspective());
    wxConfigBase::Get()->Write("OpenFiles", count);
    wxConfigBase::Get()->Write("ShowHistory", m_History != nullptr && m_History->IsShown());
    wxConfigBase::Get()->Write("ShowProjects", m_Projects != nullptr && m_Projects->IsShown());
    if (m_App->GetData().Control().Command().empty())
    {
      wxDELETE(m_Process);
    }
    event.Skip();
    });
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wxExGetVersionInfo().Get());
    wxString description(
      _("This program offers a portable text or binary editor\n"
        "with automatic syncing."));
#ifdef __WXMSW__
    description +=
      _(" All its config files are read\n"
        "and saved in the same directory as where the executable is.");
#endif
    info.SetDescription(description);
    info.SetCopyright(wxExGetVersionInfo().Copyright());
    info.SetWebSite("http://sourceforge.net/projects/syncped/");
    wxAboutBox(info);}, wxID_ABOUT);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    ShowPane("PROCESS"); 
    m_Process->Execute();}, wxID_EXECUTE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxLaunchDefaultBrowser(
      "http://antonvw.github.io/syncped/v" + 
      wxExGetVersionInfo().Get() + 
      "/syncped.htm");}, wxID_HELP);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExPrinting::Get()->GetHtmlPrinter()->PageSetup();}, wxID_PRINT_SETUP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    // In hex mode we cannot edit the file.
    if (wxConfigBase::Get()->ReadBool("HexMode", false)) return;

    static std::string name = event.GetString().ToStdString();
    wxTextEntryDialog dlg(this, _("Input") + ":", _("File Name"), name);
    wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
    validator.SetCharExcludes("/\\?%*:|\"<>");
    dlg.SetTextValidator(validator);
    if (dlg.ShowModal() == wxID_CANCEL) return;

    if (name = dlg.GetValue(); !name.empty())
    {
      auto* page = new wxExSTC(std::string(),
        wxExSTCData(m_App->GetData()).Window(wxExWindowData().
          Parent(m_Editors)));
      page->GetFile().FileNew(name);
      // This file does yet exist, so do not give it a bitmap.
      m_Editors->AddPage(page, name, name, true);
      ShowPane("FILES");}}, wxID_NEW);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Process->Kill(); 
    ShowPane("PROCESS", false);}, wxID_STOP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Editors->ForEach<wxExSTC>(event.GetId());}, 
    ID_ALL_CLOSE, ID_ALL_SAVE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    OpenFile(wxExViMacros::GetFileName());}, ID_EDIT_MACRO);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetSTC() != nullptr)
    {
      GetSTC()->ShowLineNumbers(!GetSTC()->ShownLineNumbers());
    };}, ID_EDIT_PANE_INFO_TOGGLE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetSTC() != nullptr)
    {
      wxPostEvent(GetSTC(), event);
    };}, ID_EDIT_CONTROL_CHAR);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExListView::ConfigDialog(wxExWindowData().
      Title(_("List Options").ToStdString()).
      Button(wxOK | wxCANCEL | wxAPPLY).
      Id(ID_OPTION_LIST));}, ID_OPTION_LIST);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (wxExVCS().ConfigDialog() == wxID_OK)
    { 
      wxExVCS vcs;
      vcs.SetEntryFromBase(this);
      m_StatusBar->ShowField(
        "PaneVCS", 
        vcs.Use());
      StatusText(vcs.GetName(), "PaneVCS");
    };}, ID_OPTION_VCS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (wxExProcess::ConfigDialog() == wxID_OK)
    {
      ShowPane("PROCESS");
      m_Process->Execute();
    };}, ID_PROCESS_SELECT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* project = GetProject();
      project != nullptr && m_Projects != nullptr)
    {
      m_Projects->DeletePage(project->GetFileName().Path().string());
    };}, ID_PROJECT_CLOSE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_Projects == nullptr) AddPaneProjects();
    const std::string text = wxString::Format("%s%d", _("project"), m_NewProjectNo++).ToStdString();
    const wxExPath fn(
       (!GetProjectHistory().GetHistoryFile().Path().empty() ? 
           GetProjectHistory().GetHistoryFile().GetPath(): wxExConfigDir()),
      text + ".prj");
    wxWindow* page = new wxExListViewFile(fn.Path().string(), wxExWindowData().Parent(m_Projects));
    ((wxExListViewFile*)page)->FileNew(fn.Path().string());
    // This file does yet exist, so do not give it a bitmap.
    m_Projects->AddPage(page, fn.Path().string(), text, true);
    SetRecentProject(fn.Path().string());
    ShowPane("PROJECTS");}, ID_PROJECT_NEW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxFileDialog dlg(this,
      _("Select Projects"),
       (!GetProjectHistory().GetHistoryFile().Path().empty() ? 
           GetProjectHistory().GetHistoryFile().GetPath(): wxExConfigDir()),
      wxEmptyString,
      m_ProjectWildcard,
      wxFD_OPEN | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    const std::vector < wxExPath > v(
#ifdef __WXOSX__
      {dlg.GetPath().ToStdString()});
#else
      wxExToVectorPath(dlg).Get());
#endif
    wxExOpenFiles(this, 
      v, wxExSTCData().Flags(STC_WIN_IS_PROJECT));}, 
    ID_PROJECT_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* project = GetProject(); project != nullptr)
    {
      if (wxExFileDialog(project).ShowModalIfChanged() != wxID_CANCEL)
      {
        OpenFile(project->GetFileName());
      }
    };}, 
    ID_PROJECT_OPENTEXT);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* project = GetProject();
      project != nullptr && m_Projects != nullptr)
    {
      wxExFileDialog dlg(
        project, 
        wxExWindowData().
          Style(wxFD_SAVE).
          Parent(this).
          Title(_("Project Save As").ToStdString()),
        m_ProjectWildcard);
      if (dlg.ShowModal() == wxID_OK)
      {
        project->FileSave(dlg.GetPath().ToStdString());
        m_Projects->SetPageText(
          m_Projects->GetKeyByPage(project),
          project->GetFileName().Path().string(),
          project->GetFileName().GetName());
      }
    };}, ID_PROJECT_SAVEAS);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Editors->Rearrange(wxTOP);}, ID_REARRANGE_HORIZONTALLY);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Editors->Rearrange(wxLEFT);}, ID_REARRANGE_VERTICALLY);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxConfigBase::Get()->Write("List/SortSync", 
      !wxConfigBase::Get()->ReadBool("List/SortSync", true));}, ID_SORT_SYNC);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExVCS(std::vector< wxExPath >(), event.GetId() - ID_VCS_LOWEST - 1).Request();},
    ID_VCS_LOWEST, ID_VCS_HIGHEST);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_asciiTable == nullptr)
    {
      m_asciiTable = new wxExSTC();
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
    else
    {
      TogglePane("ASCIITABLE"); 
    };}, ID_VIEW_ASCII_TABLE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("DIRCTRL");}, ID_VIEW_DIRCTRL);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("FILES"); 
    if (!GetManager().GetPane("FILES").IsShown())
    {
      if (GetManager().GetPane("PROJECTS").IsShown())
      {
        GetManager().GetPane("PROJECTS").Maximize();
        GetManager().Update();
      }
    };}, ID_VIEW_FILES);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_History == nullptr)
    {
      AddPaneHistory();
      GetManager().Update();
    }
    else
    {
      TogglePane("HISTORY");}}, ID_VIEW_HISTORY);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("OUTPUT");}, ID_VIEW_OUTPUT);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("PROJECTS");}, ID_VIEW_PROJECTS);
    
  Bind(wxEVT_SIZE, [=](wxSizeEvent& event) {
    event.Skip();
    if (IsMaximized())
    {   
      m_Maximized = true;
    }
    else if (m_Maximized)
    {
      if (m_Editors->IsSplit())
      {
        m_Editors->Rearrange(wxLEFT);
        m_Editors->Reset();
      }

      m_Maximized = false;
    };});

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(
      m_asciiTable != nullptr && 
      GetManager().GetPane("ASCIITABLE").IsShown());}, ID_VIEW_ASCII_TABLE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("DIRCTRL").IsShown());}, ID_VIEW_DIRCTRL);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("FILES").IsShown());}, ID_VIEW_FILES);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(m_History != nullptr && GetManager().GetPane("HISTORY").IsShown());}, ID_VIEW_HISTORY);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("OUTPUT").IsShown());}, ID_VIEW_OUTPUT);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(m_Projects != nullptr && GetManager().GetPane("PROJECTS").IsShown());}, ID_VIEW_PROJECTS);
  
  m_App->Reset();
}    

wxExListView* Frame::Activate(wxExListType type, const wxExLexer* lexer)
{
  if (type == LIST_FILE)
  {
    return GetProject();
  }
  else
  {
    ShowPane("OUTPUT");

    const std::string name = wxExListViewData().Type(type).TypeDescription() +
      (lexer != nullptr ?  " " + lexer->GetDisplayLexer(): std::string());
    auto* list = (wxExListViewWithFrame*)m_Lists->GetPageByKey(name);

    if (list == nullptr && type != LIST_FILE)
    {
      list = new wxExListViewWithFrame(wxExListViewData(wxExWindowData().Parent(m_Lists)).
        Type(type).
        Lexer(lexer));

      m_Lists->AddPage(list, name, name, true);
    }

    return list;
  }
}

void Frame::AddPaneHistory()
{
  m_History = new wxExListViewWithFrame(wxExListViewData().Type(LIST_HISTORY));
        
  GetManager().AddPane(m_History, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .Name("HISTORY")
    .CloseButton(false)
    .MinSize(150, 150)
    .Caption(_("History")));
}

void Frame::AddPaneProjects()
{
  m_Projects = new wxExNotebook(wxExWindowData().Id(ID_NOTEBOOK_PROJECTS).Style(m_PaneFlag));
    
  GetManager().AddPane(m_Projects, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .Name("PROJECTS")
    .MinSize(150, 150)
    .Caption(_("Projects")));
}

bool Frame::ExecExCommand(wxExExCommand& command)
{
  if (command.Command() == ":") return false;

  if (m_App->GetScriptout().IsOpened())
  {
    m_App->GetScriptout().Write(command.Command() + "\n");
  }

  bool handled = false;

  if (m_Editors->GetPageCount() > 0)
  {
    if (command.Command() == ":n")
    {
      if (m_Editors->GetSelection() == m_Editors->GetPageCount() - 1) return false;
      
      m_Editors->AdvanceSelection();
      handled = true;
    }
    else if (command.Command() == ":prev")
    {
      if (m_Editors->GetSelection() == 0) return false;
      
      m_Editors->AdvanceSelection(false);
      handled = true;
    }

    if (handled && wxExEx::GetMacros().Mode()->IsPlayback())
    {
      command.Set(((wxExSTC *)m_Editors->GetPage(
        m_Editors->GetSelection()))->GetVi().GetCommand());
    }
  }

  return handled;
}

wxExListViewFile* Frame::GetProject()
{
  if (m_Projects == nullptr)
  {
    return nullptr;
  }

  if (!m_Projects->IsShown() || 
       m_Projects->GetPageCount() == 0)
  {
    return nullptr;
  }
  else
  {
    return (wxExListViewFile*)m_Projects->
      GetPage(m_Projects->GetSelection());
  }
}

bool Frame::IsOpen(const wxExPath& filename)
{
  return m_Editors->GetPageIndexByKey(filename.Path().string()) != wxNOT_FOUND;
}

void Frame::OnCommand(wxCommandEvent& event)
{
  auto* editor = GetSTC();

  switch (event.GetId())
  {
  // edit commands
  // Do not change the wxID* in wxID_LOWEST and wdID_HIGHEST,
  // as wxID_ABOUT etc. is used here and not in the editor.
  // That causes appl to hang.
  case wxID_UNDO:
  case wxID_REDO:
  case wxID_CLEAR:
  case wxID_COPY:
  case wxID_CUT:
  case wxID_DELETE:
  case wxID_JUMP_TO:  
  case wxID_PASTE:
  case wxID_SELECTALL:
    if (editor != nullptr)
    {
      wxPostEvent(editor, event);
    }
    else if (GetListView() != nullptr)
    {
      wxPostEvent(GetListView(), event);
    }
  break;

  case wxID_CLOSE:
    if (editor != nullptr)
    {
      if (!AllowClose(m_Editors->GetId(), editor)) return;
      m_Editors->DeletePage(m_Editors->GetKeyByPage(editor));
    }
    break;
  case wxID_PREVIEW:
    if (editor != nullptr)
    {
      editor->PrintPreview();
    }
    else if (GetListView() != nullptr)
    {
      GetListView()->PrintPreview();
    }
    break;
  case wxID_PRINT:
    if (editor != nullptr)
    {
      editor->Print();
    }
    else if (GetListView() != nullptr)
    {
      GetListView()->Print();
    }
    break;
  case wxID_SAVE:
    if (editor != nullptr)
    {
      if (!editor->IsModified() || !editor->GetFile().FileSave()) return;

      SetRecentFile(editor->GetFileName());
      
      if (editor->GetFileName() == wxExLexers::Get()->GetFileName())
      {
        if (wxExLexers::Get()->LoadDocument())
        {
          m_Editors->ForEach<wxExSTC>(ID_ALL_STC_SET_LEXER);

          // As the lexer might have changed, update status bar field as well.
          UpdateStatusBar(editor, "PaneLexer");
        }
      }
      else if (editor->GetFileName() == wxExMenus::GetFileName())
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
    if (editor != nullptr)
    {
      if (!event.GetString().empty())
      {
        if (!editor->GetFile().FileSave(event.GetString().ToStdString()))
        {
          return;
        }
      }
      else
      {
        wxExFileDialog dlg(
          &editor->GetFile(), 
          wxExWindowData().
            Style(wxFD_SAVE). 
            Parent(this).
            Title(wxGetStockLabel(wxID_SAVEAS, wxSTOCK_NOFLAGS).ToStdString()));

        if (dlg.ShowModal() != wxID_OK)
        {
          return;
        }

        if (!editor->GetFile().FileSave(dlg.GetPath().ToStdString()))
        {
          return;
        }
      }
      
      const wxBitmap bitmap = (editor->GetFileName().GetStat().IsOk() ? 
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(wxExGetIconID(editor->GetFileName())) : 
        wxNullBitmap);

      m_Editors->SetPageText(
        m_Editors->GetKeyByPage(editor),
        editor->GetFileName().Path().string(),
        editor->GetFileName().GetFullName(),
        bitmap);
          
      editor->PropertiesMessage();
      
      SetRecentFile(editor->GetFileName());
    }
    break;

  case ID_EDIT_MACRO_PLAYBACK: 
    if (editor != nullptr) 
      editor->GetVi().GetMacros().Mode()->Transition("@", &editor->GetVi(), true); break;
  case ID_EDIT_MACRO_START_RECORD: 
  case ID_EDIT_MACRO_STOP_RECORD: 
    if (editor != nullptr) 
      editor->GetVi().GetMacros().Mode()->Transition("q", &editor->GetVi(), true); break;
  
  case ID_SPLIT:
  case ID_SPLIT_HORIZONTALLY:
  case ID_SPLIT_VERTICALLY:
    if (editor == nullptr)
    {
      wxLogStatus("no valid focus");
    }
    else
    {
      auto* stc = new wxExSTC(editor->GetFileName(), 
        wxExSTCData().Window(wxExWindowData().Parent(m_Editors)));
      editor->Sync(false);
      stc->Sync(false);
      stc->GetVi().Copy(&editor->GetVi());

      wxBitmap bitmap(wxNullBitmap);
      
      if (editor->GetFileName().FileExists())
      {
        bitmap = wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wxExGetIconID(editor->GetFileName()));
      }
      else if (!editor->GetLexer().GetScintillaLexer().empty())
      {
        stc->GetLexer().Set(editor->GetLexer().GetScintillaLexer());
      }
      
      // key should be unique
      const std::string key("split" + std::to_string(m_SplitId++));
      
      // Place new page before page for editor.
      m_Editors->InsertPage(
        m_Editors->GetPageIndex(editor),
        stc,
        key,
        editor->GetFileName().GetFullName(),
        true,
        bitmap);

      stc->SetDocPointer(editor->GetDocPointer());
      
      if (event.GetId() == ID_SPLIT_HORIZONTALLY)
      {
        m_Editors->Split(key, wxBOTTOM);
        m_Editors->SetSelection(editor->GetFileName().Path().string());
      }
      else if (event.GetId() == ID_SPLIT_VERTICALLY)
      {
        m_Editors->Split(key, wxRIGHT);
        m_Editors->SetSelection(editor->GetFileName().Path().string());
      }
    }
    break;
    
  default: 
      wxFAIL;
    break;
  }
}

void Frame::OnCommandItemDialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  switch (dialogid)
  {
    case wxID_PREFERENCES:
      if (event.GetId() != wxID_CANCEL)
      {
        m_Editors->ForEach<wxExSTC>(ID_ALL_CONFIG_GET);
        
        if (m_Process->GetShell() != nullptr)
        {
          m_Process->GetShell()->ConfigGet();
        }
        
        m_StatusBar->ShowField(
          "PaneMacro", 
          wxConfigBase::Get()->ReadBool(_("vi mode"), true));
      }
      break;

    case ID_OPTION_LIST:
      if (event.GetId() != wxID_CANCEL)
      {
        m_Lists->ForEach<wxExListViewFile>(ID_ALL_CONFIG_GET);
        if (m_Projects != nullptr) m_Projects->ForEach<wxExListViewFile>(ID_ALL_CONFIG_GET);
        if (m_History != nullptr) m_History->ConfigGet();
      }
      break;
    
    default:
      DecoratedFrame::OnCommandItemDialog(dialogid, event);
  }
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
    case wxID_EXECUTE: 
      event.Enable( !IsClosing() && m_Process != nullptr &&
		            !m_Process->GetExecuteCommand().empty() &&
                    !m_Process->IsRunning()); 
      break;
    case wxID_STOP: event.Enable(m_Process->IsRunning()); break;
    case wxID_PREVIEW:
    case wxID_PRINT:
      event.Enable(
        (GetSTC() != nullptr && GetSTC()->GetLength() > 0) ||
        (GetListView() != nullptr && GetListView()->GetItemCount() > 0));
      break;

    case wxID_CLOSE:
    case wxID_SAVEAS:
      event.Enable(m_Editors->IsShown() && m_Editors->GetPageCount() > 0);
    break;
    
    case ID_ALL_CLOSE:
    case ID_ALL_SAVE:
      event.Enable(m_Editors->GetPageCount() > 2);
    break;

    case ID_OPTION_VCS:
      event.Enable(wxExVCS::GetCount() > 0);
      break;

    case ID_PROJECT_CLOSE:
    case ID_PROJECT_SAVEAS:
      event.Enable(GetProject() != nullptr && GetProject()->IsShown());
      break;
    case ID_PROJECT_OPENTEXT:
      event.Enable(
        GetProject() != nullptr && !GetProject()->GetFileName().Path().empty());
      break;
    case ID_PROJECT_SAVE:
      event.Enable(GetProject() != nullptr && GetProject()->GetContentsChanged());
      break;

    case ID_RECENT_FILE_MENU:
      event.Enable(!GetFileHistory().GetHistoryFile().Path().empty());
      break;
    case ID_RECENT_PROJECT_MENU:
      event.Enable(!GetProjectHistory().GetHistoryFile().Path().empty());
      break;

    case ID_SORT_SYNC:
      event.Check(wxConfigBase::Get()->ReadBool("List/SortSync", true));
      break;

    default:
    {
      if (auto* editor = GetSTC(); editor != nullptr)
      {
        event.Enable(true);

        switch (event.GetId())
        {
        case wxID_FIND:
        case wxID_JUMP_TO:
        case wxID_REPLACE:
        case ID_EDIT_FIND_NEXT:
        case ID_EDIT_FIND_PREVIOUS:
          event.Enable(editor->GetLength() > 0);
          break;
        case ID_EDIT_MACRO:
          event.Enable(
             editor->GetVi().GetIsActive() &&
            !editor->GetVi().GetMacros().Mode()->IsRecording() &&
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
            !editor->GetVi().GetMacros().Mode()->IsRecording());
          break;
        case ID_EDIT_MACRO_START_RECORD:
          event.Enable(
             editor->GetVi().GetIsActive() && 
            !editor->GetVi().GetMacros().Mode()->IsRecording());
          break;
        case ID_EDIT_MACRO_STOP_RECORD:
          event.Enable(editor->GetVi().GetMacros().Mode()->IsRecording());
          break;

        case wxID_SAVE:
          event.Enable(
            !editor->GetFileName().Path().empty() &&
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
      else if (auto* list = (wxExListViewFile*)GetListView();
        list != nullptr && list->IsShown())
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

wxExSTC* Frame::OpenFile(
  const wxExPath& filename,
  const wxExVCSEntry& vcs,
  const wxExSTCData& data)
{
  if (vcs.GetCommand().IsBlame())
  {
    if (auto* page = (wxExSTC*)m_Editors->SetSelection(filename.Path().string());
      page != nullptr)
    {
      if (page->ShowVCS(&vcs)) return page;
    }
  }

  const std::string unique = 
    vcs.GetCommand().GetCommand() + " " + vcs.GetFlags();
  const std::string key = filename.Path().string() + " " + unique;

  auto* page = (wxExSTC*)m_Editors->SetSelection(key);
  
  if (page == nullptr)
  {
    page = new wxExSTC(
      vcs.GetStdOut(),
      wxExSTCData(data).Window(wxExWindowData().
        Parent(m_Editors).
        Name(filename.GetFullName() + " " + unique)));

    wxExVCSCommandOnSTC(
      vcs.GetCommand(), filename.GetLexer(), page);
      
    if (const int index = m_Editors->GetPageIndexByKey(filename.Path().string());
      index != -1)
    {
      // Place new page before the one used for vcs.
      m_Editors->InsertPage(index, page, key, filename.GetFullName() + " " + unique, true);
    }
    else
    {
      // Just add at the end.
      m_Editors->AddPage(page, key, filename.GetFullName() + " " + unique, true);
    }
  }

  return page;
}

wxExSTC* Frame::OpenFile(
  const wxExPath& filename,
  const std::string& text,
  const wxExSTCData& data)
{
  auto* page = (wxExSTC*)m_Editors->SetSelection(filename.Path().string());

  if (page == nullptr)
  {
    page = new wxExSTC(text, 
      wxExSTCData(data).Window(wxExWindowData().
        Parent(m_Editors).
        Name(filename.Path().string())));
    page->GetLexer().Set(filename.GetLexer());
    m_Editors->AddPage(page, filename.Path().string(), filename.GetFullName(), true);
  }
  else
  {
    page->SetText(text);
  }

  return page;
}
  
wxExSTC* Frame::OpenFile(const wxExPath& filename, const wxExSTCData& data)
{
  if ((data.Flags() & STC_WIN_IS_PROJECT) && m_Projects == nullptr)
  {
    AddPaneProjects();
    GetManager().Update();
  }
  
  wxExNotebook* notebook = ((data.Flags() & STC_WIN_IS_PROJECT)
    ? m_Projects : m_Editors);
    
  wxASSERT(notebook != nullptr);
  
  wxWindow* page = notebook->SetSelection(filename.Path().string());

  if (data.Flags() & STC_WIN_IS_PROJECT)
  {
    if (page == nullptr)
    {
      auto* project = new wxExListViewFile(filename.Path().string(), wxExWindowData().Parent(m_Projects));

      notebook->AddPage(
        project,
        filename.Path().string(),
        filename.GetName(),
        true,
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wxExGetIconID(filename)));
    }

    if (!GetManager().GetPane("PROJECTS").IsShown())
    {
      ShowPane("PROJECTS");
    }
  }
  else
  {
    if (!GetManager().GetPane("FILES").IsShown())
    {
      if (GetManager().GetPane("PROJECTS").IsMaximized())
      {
        GetManager().GetPane("PROJECTS").Restore();
      }

      ShowPane("FILES");
    }

    if (filename == wxExViMacros::GetFileName())
    {
      wxExViMacros::SaveDocument();
    }
    
    auto* editor = (wxExSTC*)page;

    if (page == nullptr)
    {
      editor = new wxExSTC(filename, 
        wxExSTCData(data).
          Window(wxExWindowData().Parent(m_Editors)).
          Flags(m_App->GetData().Flags(), DATA_OR).
          Flags(wxConfigBase::Get()->ReadBool("HexMode", false) ? STC_WIN_HEX: STC_WIN_DEFAULT, DATA_OR).
          Menu(m_App->GetDebug() ? STC_MENU_DEBUG: STC_MENU_NONE, DATA_OR));
      
      if (m_App->GetDebug())
      {
        for (const auto& it: GetDebug()->GetBreakpoints())
        {
          if (std::get<0>(it.second) == filename)
          {
            editor->MarkerAdd(std::get<2>(it.second), 
              GetDebug()->GetMarkerBreakpoint().GetNo());
          }
        }
      }

      const std::string key(filename.Path().string());

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
      
      if (GetManager().GetPane("DIRCTRL").IsShown())
      {
        m_DirCtrl->ExpandAndSelectPath(key);
      }
      
      // Do not show an edge for project files opened as text.
      if (filename.GetExtension() == ".prj")
      {
        editor->SetEdgeMode(wxSTC_EDGE_NONE);
      }
      
      if (m_App->GetScriptin().IsOpened())
      {
        const auto buffer(m_App->GetScriptin().Read());
        wxExTokenizer tkz(std::string((const char *)buffer->data(), buffer->length()), "\r\n");
        while (tkz.HasMoreTokens())
        {
          if (!editor->GetVi().Command( tkz.GetNextToken()))
          {
            wxLogStatus("Aborted at: %s", tkz.GetToken().c_str());
            return editor;
          }
        }
      }
    }
    else
    {
      wxExSTCData(editor, data).Inject();
    }
    
    editor->SetFocus();
    
    return editor;
  }
  
  return (wxExSTC*)page;
}

void Frame::PrintEx(wxExEx* ex, const std::string& text)
{
  auto* page = (wxExSTC*)m_Editors->SetSelection("Print");

  if (page == nullptr)
  {
    page = new wxExSTC(text, wxExSTCData().
      Window(wxExWindowData().Name("Print").Parent(m_Editors)));
    m_Editors->AddPage(page, "Print", "Print", true);
    m_Editors->Split("Print", wxBOTTOM);
  }
  else
  {
    page->AppendText(text);
    page->DocumentEnd();
    page->SetSavePoint();
  }
  
  page->GetLexer().Set(ex->GetSTC()->GetLexer());
}
  
wxExProcess* Frame::Process(const std::string& command)
{
  m_Process->Execute(command, PROCESS_EXEC_DEFAULT);
  return m_Process;
}

wxExSTC* Frame::RestorePage(const std::string& key)
{
  if (!m_SavedPage.empty() && IsOpen(m_SavedPage))
  {
    m_Editors->ChangeSelection(m_SavedPage);
    return (wxExSTC *)m_Editors->GetPageByKey(m_SavedPage);
  }
  
  return nullptr;
}
  
bool Frame::SaveCurrentPage(const std::string& key)
{
  m_SavedPage = m_Editors->GetCurrentPage();
  return true;
}

void Frame::StatusBarClicked(const std::string& pane)
{
  if (pane == "PaneTheme")
  {
    if (wxExLexers::Get()->ShowThemeDialog(m_Editors))
    {
      m_Editors->ForEach<wxExSTC>(ID_ALL_STC_SET_LEXER_THEME);

      m_StatusBar->ShowField(
        "PaneLexer", 
        !wxExLexers::Get()->GetTheme().empty());
        
      StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
    }
  }
  else if (pane == "PaneMacro")
  {
    if (auto* editor = GetSTC(); editor != nullptr) 
      editor->GetVi().GetMacros().Mode()->Transition("@", &editor->GetVi(), true);
  }
  else if (pane == "PaneVCS")
  {
    if (wxExVCS::GetCount() > 0)
    {
      auto* menu = new wxExMenu;
      
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

void Frame::StatusBarClickedRight(const std::string& pane)
{
  if (pane == "PaneInfo")
  {
    if (auto* stc = GetSTC(); stc != nullptr)
    {
      wxMenu* menu = new wxMenu();
      menu->Append(ID_EDIT_PANE_INFO_TOGGLE, 
        stc->ShownLineNumbers() ? "&Hide": "&Show");
      PopupMenu(menu);
    }
  }
  else if (pane == "PaneLexer" || pane == "PaneTheme")
  {
    std::string match;
    
    if (pane == "PaneLexer")
    {
      if (auto* stc = GetSTC(); stc != nullptr)
      {
        if (
          !stc->GetLexer().GetScintillaLexer().empty() && 
           stc->GetLexer().GetScintillaLexer() == stc->GetLexer().GetDisplayLexer())
        {
          match = "lexer *name *= *\"" + stc->GetLexer().GetScintillaLexer() + "\"";
        }
        else if (!stc->GetLexer().GetDisplayLexer().empty())
        {
          match = "display *= *\"" + stc->GetLexer().GetDisplayLexer() + "\"";
        }
        else
        {
          return;
        }
      }
    }
    else
    {
      if (wxExLexers::Get()->GetTheme().empty())
      {
        return;
      }
      
      match = "theme *name *= *\"" + wxExLexers::Get()->GetTheme() + "\"";
    }
    
    OpenFile(
      wxExLexers::Get()->GetFileName(), 
      wxExControlData().Find(match, wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX));
  }
  else if (pane == "PaneMacro")
  {
    if (wxExViMacros::GetFileName().FileExists())
    {
      OpenFile(wxExViMacros::GetFileName(),
        wxExControlData().Find(!GetStatusText(pane).empty() ? " name=\"" + GetStatusText(pane) + "\"":
          std::string()));
    }
  }
  else if (pane == "PaneVCS")
  {
    std::string match(GetStatusText(pane));

    if (auto* stc = GetSTC(); stc != nullptr)
    {
      const wxExVCS vcs({stc->GetFileName().Path().string()});
      match = vcs.GetEntry().GetName();
    }

    OpenFile(wxExMenus::GetFileName(), wxExControlData().Find(match));
  }
  else
  {
    DecoratedFrame::StatusBarClickedRight(pane);
  }
}

void Frame::SyncAll()
{
  m_Editors->ForEach<wxExSTC>(ID_ALL_STC_SYNC);
}

void Frame::SyncCloseAll(wxWindowID id)
{
  DecoratedFrame::SyncCloseAll(id);
  
  if (IsClosing()) return;
  
  switch (id)
  {
  case ID_NOTEBOOK_EDITORS:
    SetTitle(wxTheApp->GetAppDisplayName());
    StatusText(std::string(), std::string());
    StatusText(std::string(), "PaneFileType");
    StatusText(std::string(), "PaneInfo");
    StatusText(std::string(), "PaneLexer");
    
    if (GetManager().GetPane("PROJECTS").IsShown() && m_Projects != nullptr)
    {
      GetManager().GetPane("PROJECTS").Maximize();
      GetManager().Update();
    }
    break;
  case ID_NOTEBOOK_LISTS: ShowPane("OUTPUT", false); break;
  case ID_NOTEBOOK_PROJECTS: ShowPane("PROJECTS", false); break;
  default: wxFAIL;
  }
}

EditorsNotebook::EditorsNotebook(const wxExWindowData& data)
  : wxExNotebook(data)
{
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxPostEvent(wxAuiNotebook::GetCurrentPage(), event);
    }, ID_EDIT_VCS_LOWEST, ID_EDIT_VCS_HIGHEST);

  Bind(wxEVT_AUINOTEBOOK_END_DRAG, [=](wxAuiNotebookEvent& event) {
    event.Skip();
    m_Split = true;
    });

  Bind(wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, [=](wxAuiNotebookEvent& event) {
    wxExMenu menu;
    
    auto* split = new wxExMenu;
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

    menu.AppendSubMenu(split, _("Split"), std::string(), ID_SPLIT_MENU);
    menu.AppendSeparator();
    menu.Append(wxID_CLOSE);
    menu.Append(ID_ALL_CLOSE, _("Close A&ll"));
    
    if (GetPageCount() > 2)
    {
      menu.Append(ID_ALL_CLOSE_OTHERS, _("Close Others"));
    }

    if (auto* stc = wxDynamicCast(wxAuiNotebook::GetCurrentPage(), wxExSTC);
      stc->GetFile().GetFileName().FileExists() && 
        wxExVCS::DirExists(stc->GetFile().GetFileName()))
    {
      menu.AppendSeparator();
      menu.AppendVCS(stc->GetFile().GetFileName());
    }
    
    PopupMenu(&menu);});
}
