////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of class frame
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
#include <wex/ctags.h>
#include <wex/debug.h>
#include <wex/filedlg.h>
#include <wex/itemdlg.h>
#include <wex/lexers.h>
#include <wex/menu.h>
#include <wex/menus.h>
#include <wex/otl.h>
#include <wex/printing.h>
#include <wex/shell.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/tostring.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wex/version.h>
#include <wex/vi-macros.h>
#include <wex/vi-macros-mode.h>
#include <wex/report/listviewfile.h>
#include "frame.h"
#include "app.h"
#include "defs.h"

const int ID_EDIT_PANE_INFO_TOGGLE = wxWindow::NewControlId();

class editors : public wex::notebook
{
public:
  editors(const wex::window_data& data); 

  /// Returns true if notebook is splitted.
  bool IsSplit() const {return m_Split;};

  /// Reset members.
  void Reset() {m_Split = false;};
protected:
  bool m_Split {false};
};

BEGIN_EVENT_TABLE(frame, decorated_frame)
  EVT_MENU(wxID_DELETE, frame::OnCommand)
  EVT_MENU(wxID_EXECUTE, frame::OnCommand)
  EVT_MENU(wxID_JUMP_TO, frame::OnCommand)
  EVT_MENU(wxID_SELECTALL, frame::OnCommand)
  EVT_MENU(wxID_STOP, frame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, frame::OnCommand)
  EVT_MENU_RANGE(wxID_CLOSE, wxID_CLOSE_ALL, frame::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_MACRO_PLAYBACK, ID_EDIT_MACRO_STOP_RECORD, frame::OnCommand)
  EVT_MENU_RANGE(ID_SPLIT, ID_SPLIT_VERTICALLY, frame::OnCommand)
  EVT_UPDATE_UI(wex::ID_ALL_CLOSE, frame::OnUpdateUI)
  EVT_UPDATE_UI(wex::ID_ALL_SAVE, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_CLOSE, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_EXECUTE, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_FIND, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_JUMP_TO, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PRINT, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PREVIEW, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REPLACE, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UNDO, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_REDO, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_SAVE, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_SAVEAS, frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_STOP, frame::OnUpdateUI)
  EVT_UPDATE_UI(wex::ID_EDIT_CONTROL_CHAR, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_MENU, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_PLAYBACK, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_START_RECORD, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_EDIT_MACRO_STOP_RECORD, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_OPTION_VCS, frame::OnUpdateUI)
  EVT_UPDATE_UI(wex::ID_PROJECT_SAVE, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_FILE_MENU, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_PROJECT_MENU, frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SORT_SYNC, frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(wex::ID_EDIT_FIND_NEXT, wex::ID_EDIT_FIND_PREVIOUS, frame::OnUpdateUI)
  EVT_UPDATE_UI_RANGE(ID_PROJECT_OPENTEXT, ID_PROJECT_SAVEAS, frame::OnUpdateUI)
END_EVENT_TABLE()

frame::frame(app* app)
  : decorated_frame(app)
  , m_Process(new wex::process())
  , m_Editors(new editors(wex::window_data().Id(wex::ID_NOTEBOOK_EDITORS).Style(m_PaneFlag)))
  , m_Lists(new wex::notebook(
      wex::window_data().Id(wex::ID_NOTEBOOK_LISTS).Style(m_PaneFlag)))
  , m_DirCtrl(new wex::dirctrl(this))
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
    wex::ctags(this).Find(m_App->GetTag());
  }
  else if (m_App->GetFiles().empty())
  {
    if (long count = 0; 
      wxConfigBase::Get()->Read("OpenFiles", &count) && count > 0)
    {
      wex::open_files(this, GetFileHistory().GetHistoryFiles(count), 
        m_App->GetData());
    }
      
    if (GetManager().GetPane("PROJECTS").IsShown() && m_Projects != nullptr)
    {
      if (!GetProjectHistory().GetHistoryFile().Path().empty())
      {
        OpenFile(
          wex::path(GetProjectHistory().GetHistoryFile()),
          wex::stc_data().Flags(wex::stc_data::WIN_IS_PROJECT));
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
    wex::open_files(this, m_App->GetFiles(), m_App->GetData(), wex::dir::FILES);
  }
  
  StatusText(wex::lexers::Get()->GetTheme(), "PaneTheme");
  
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
    GetFileHistory().PopupMenu(this, wex::ID_CLEAR_FILES);}, wex::ID_NOTEBOOK_EDITORS);
    
  Bind(wxEVT_AUINOTEBOOK_BG_DCLICK, [=] (wxAuiNotebookEvent& event) {
    GetProjectHistory().PopupMenu(this, wex::ID_CLEAR_PROJECTS);}, wex::ID_NOTEBOOK_PROJECTS);
    
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
    GetDebug()->Execute(event.GetId() - wex::ID_EDIT_DEBUG_FIRST);}, 
    wex::ID_EDIT_DEBUG_FIRST, wex::ID_EDIT_DEBUG_LAST);
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    long count = 0;
    for (size_t i = 0; i < m_Editors->GetPageCount(); i++)
    {
      if (auto* stc = wxDynamicCast(m_Editors->GetPage(i), wex::stc);
        stc->GetFileName().FileExists())
      {
        count++;
      }
    }
    if (event.CanVeto())
    {
      if ((m_Process->IsRunning() && m_Process->GetExecuteCommand() != "gdb") || 
        !m_Editors->ForEach<wex::stc>(wex::ID_ALL_CLOSE) || 
        (m_Projects != nullptr && !m_Projects->ForEach<wex::listview_file>(wex::ID_ALL_CLOSE)))
      {
        event.Veto();
        if (m_Process->IsRunning())
        {
          wxLogStatus(_("Process is running"));
        }
        return;
      }
    }
    wex::vi_macros::SaveDocument();
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
    info.SetVersion(wex::get_version_info().Get());
    wxString description(
      _("This program offers a portable text or binary editor\n"
        "with automatic syncing."));
#ifdef __WXMSW__
    description +=
      _(" All its config files are read\n"
        "and saved in the same directory as where the executable is.");
#endif
    info.SetDescription(description);
    info.SetCopyright(wex::get_version_info().Copyright());
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
      wex::get_version_info().Get() + 
      "/syncped.htm");}, wxID_HELP);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::printing::Get()->GetHtmlPrinter()->PageSetup();}, wxID_PRINT_SETUP);

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
      auto* page = new wex::stc(std::string(),
        wex::stc_data(m_App->GetData()).Window(wex::window_data().
          Parent(m_Editors)));
      page->GetFile().FileNew(name);
      // This file does yet exist, so do not give it a bitmap.
      m_Editors->AddPage(page, name, name, true);
      ShowPane("FILES");}}, wxID_NEW);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Process->Kill(); 
    ShowPane("PROCESS", false);}, wxID_STOP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Editors->ForEach<wex::stc>(event.GetId());}, 
    wex::ID_ALL_CLOSE, wex::ID_ALL_SAVE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    OpenFile(wex::vi_macros::GetFileName());}, ID_EDIT_MACRO);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetSTC() != nullptr)
    {
      GetSTC()->ShowLineNumbers(!GetSTC()->ShownLineNumbers());
    };}, ID_EDIT_PANE_INFO_TOGGLE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetSTC() != nullptr)
    {
      wxPostEvent(GetSTC(), event);
    };}, wex::ID_EDIT_CONTROL_CHAR);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::listview::ConfigDialog(wex::window_data().
      Title(_("List Options").ToStdString()).
      Button(wxOK | wxCANCEL | wxAPPLY).
      Id(ID_OPTION_LIST));}, ID_OPTION_LIST);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (wex::vcs().ConfigDialog() == wxID_OK)
    { 
      wex::vcs vcs;
      vcs.SetEntryFromBase(this);
      m_StatusBar->ShowField(
        "PaneVCS", 
        vcs.Use());
      StatusText(vcs.GetName(), "PaneVCS");
    };}, ID_OPTION_VCS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (wex::process::ConfigDialog() == wxID_OK)
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
    const wex::path fn(
       (!GetProjectHistory().GetHistoryFile().Path().empty() ? 
           GetProjectHistory().GetHistoryFile().GetPath(): wex::config().dir()),
      text + ".prj");
    wxWindow* page = new wex::listview_file(fn.Path().string(), wex::window_data().Parent(m_Projects));
    ((wex::listview_file*)page)->FileNew(fn.Path().string());
    // This file does yet exist, so do not give it a bitmap.
    m_Projects->AddPage(page, fn.Path().string(), text, true);
    SetRecentProject(fn.Path().string());
    ShowPane("PROJECTS");}, ID_PROJECT_NEW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxFileDialog dlg(this,
      _("Select Projects"),
       (!GetProjectHistory().GetHistoryFile().Path().empty() ? 
           GetProjectHistory().GetHistoryFile().GetPath(): wex::config().dir()),
      wxEmptyString,
      m_ProjectWildcard,
      wxFD_OPEN | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    const std::vector < wex::path > v(
#ifdef __WXOSX__
      {dlg.GetPath().ToStdString()});
#else
      wex::to_vector_path(dlg).Get());
#endif
    wex::open_files(this, 
      v, wex::stc_data().Flags(wex::stc_data::WIN_IS_PROJECT));}, 
    ID_PROJECT_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* project = GetProject(); project != nullptr)
    {
      if (wex::file_dialog(project).ShowModalIfChanged() != wxID_CANCEL)
      {
        OpenFile(project->GetFileName());
      }
    };}, 
    ID_PROJECT_OPENTEXT);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* project = GetProject();
      project != nullptr && m_Projects != nullptr)
    {
      wex::file_dialog dlg(
        project, 
        wex::window_data().
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
    wex::vcs(std::vector< wex::path >(), event.GetId() - wex::ID_VCS_LOWEST - 1).Request();},
    wex::ID_VCS_LOWEST, wex::ID_VCS_HIGHEST);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_asciiTable == nullptr)
    {
      m_asciiTable = new wex::stc();
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

wex::listview* frame::Activate(wex::listview_data::type type, const wex::lexer* lexer)
{
  if (type == wex::listview_data::FILE)
  {
    return GetProject();
  }
  else
  {
    ShowPane("OUTPUT");

    const std::string name = wex::listview_data().Type(type).TypeDescription() +
      (lexer != nullptr ?  " " + lexer->GetDisplayLexer(): std::string());
    auto* list = (wex::history_listview*)m_Lists->GetPageByKey(name);

    if (list == nullptr && type != wex::listview_data::FILE)
    {
      list = new wex::history_listview(wex::listview_data(wex::window_data().Parent(m_Lists)).
        Type(type).
        Lexer(lexer));

      m_Lists->AddPage(list, name, name, true);
    }

    return list;
  }
}

void frame::AddPaneHistory()
{
  m_History = new wex::history_listview(wex::listview_data().Type(wex::listview_data::HISTORY));
        
  GetManager().AddPane(m_History, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .Name("HISTORY")
    .CloseButton(false)
    .MinSize(150, 150)
    .Caption(_("History")));
}

void frame::AddPaneProjects()
{
  m_Projects = new wex::notebook(wex::window_data().Id(wex::ID_NOTEBOOK_PROJECTS).Style(m_PaneFlag));
    
  GetManager().AddPane(m_Projects, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .Name("PROJECTS")
    .MinSize(150, 150)
    .Caption(_("Projects")));
}

bool frame::ExecExCommand(wex::ex_command& command)
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

    if (handled && wex::ex::GetMacros().Mode()->IsPlayback())
    {
      command.Set(((wex::stc *)m_Editors->GetPage(
        m_Editors->GetSelection()))->GetVi().GetCommand());
    }
  }

  return handled;
}

wex::listview_file* frame::GetProject()
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
    return (wex::listview_file*)m_Projects->
      GetPage(m_Projects->GetSelection());
  }
}

bool frame::IsOpen(const wex::path& filename)
{
  return m_Editors->GetPageIndexByKey(filename.Path().string()) != wxNOT_FOUND;
}

void frame::OnCommand(wxCommandEvent& event)
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
      
      if (editor->GetFileName() == wex::lexers::Get()->GetFileName())
      {
        if (wex::lexers::Get()->LoadDocument())
        {
          m_Editors->ForEach<wex::stc>(wex::ID_ALL_STC_SET_LEXER);

          // As the lexer might have changed, update status bar field as well.
          UpdateStatusBar(editor, "PaneLexer");
        }
      }
      else if (editor->GetFileName() == wex::menus::GetFileName())
      {
        wex::vcs::LoadDocument();
      }
      else if (editor->GetFileName() == wex::vi_macros::GetFileName())
      {
        wex::vi_macros::LoadDocument();
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
        wex::file_dialog dlg(
          &editor->GetFile(), 
          wex::window_data().
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
      
      const wxBitmap bitmap = (editor->GetFileName().GetStat().is_ok() ? 
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(wex::get_iconid(editor->GetFileName())) : 
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
      auto* stc = new wex::stc(editor->GetFileName(), 
        wex::stc_data().Window(wex::window_data().Parent(m_Editors)));
      editor->Sync(false);
      stc->Sync(false);
      stc->GetVi().Copy(&editor->GetVi());

      wxBitmap bitmap(wxNullBitmap);
      
      if (editor->GetFileName().FileExists())
      {
        bitmap = wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wex::get_iconid(editor->GetFileName()));
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

void frame::OnCommandItemDialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  switch (dialogid)
  {
    case wxID_PREFERENCES:
      if (event.GetId() != wxID_CANCEL)
      {
        m_Editors->ForEach<wex::stc>(wex::ID_ALL_CONFIG_GET);
        
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
        m_Lists->ForEach<wex::listview_file>(wex::ID_ALL_CONFIG_GET);
        if (m_Projects != nullptr) m_Projects->ForEach<wex::listview_file>(wex::ID_ALL_CONFIG_GET);
        if (m_History != nullptr) m_History->ConfigGet();
      }
      break;
    
    default:
      decorated_frame::OnCommandItemDialog(dialogid, event);
  }
}

void frame::OnUpdateUI(wxUpdateUIEvent& event)
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
    
    case wex::ID_ALL_CLOSE:
    case wex::ID_ALL_SAVE:
      event.Enable(m_Editors->GetPageCount() > 2);
    break;

    case ID_OPTION_VCS:
      event.Enable(wex::vcs::GetCount() > 0);
      break;

    case ID_PROJECT_CLOSE:
    case ID_PROJECT_SAVEAS:
      event.Enable(GetProject() != nullptr && GetProject()->IsShown());
      break;
    case ID_PROJECT_OPENTEXT:
      event.Enable(
        GetProject() != nullptr && !GetProject()->GetFileName().Path().empty());
      break;
    case wex::ID_PROJECT_SAVE:
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
        case wex::ID_EDIT_FIND_NEXT:
        case wex::ID_EDIT_FIND_PREVIOUS:
          event.Enable(editor->GetLength() > 0);
          break;
        case ID_EDIT_MACRO:
          event.Enable(
             editor->GetVi().GetIsActive() &&
            !editor->GetVi().GetMacros().Mode()->IsRecording() &&
             wex::vi_macros::GetFileName().FileExists());
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
          
        case wex::ID_EDIT_CONTROL_CHAR:
          if (editor->GetReadOnly() && editor->GetSelectedText().length() != 1)
          {
            event.Enable(false);
          }
          break;

        default:
          wxFAIL;
        }
      }
      else if (auto* list = (wex::listview_file*)GetListView();
        list != nullptr && list->IsShown())
      {
        event.Enable(false);

        if (
          event.GetId() > wex::ID_TOOL_LOWEST &&
          event.GetId() < wex::ID_TOOL_HIGHEST)
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

wex::stc* frame::OpenFile(
  const wex::path& filename,
  const wex::vcs_entry& vcs,
  const wex::stc_data& data)
{
  if (vcs.GetCommand().IsBlame())
  {
    if (auto* page = (wex::stc*)m_Editors->SetSelection(filename.Path().string());
      page != nullptr)
    {
      if (page->ShowVCS(&vcs)) return page;
    }
  }

  const std::string unique = 
    vcs.GetCommand().GetCommand() + " " + vcs.GetFlags();
  const std::string key = filename.Path().string() + " " + unique;

  auto* page = (wex::stc*)m_Editors->SetSelection(key);
  
  if (page == nullptr)
  {
    page = new wex::stc(
      vcs.GetStdOut(),
      wex::stc_data(data).Window(wex::window_data().
        Parent(m_Editors).
        Name(filename.GetFullName() + " " + unique)));

    wex::vcs_command_stc(
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

wex::stc* frame::OpenFile(
  const wex::path& filename,
  const std::string& text,
  const wex::stc_data& data)
{
  auto* page = (wex::stc*)m_Editors->SetSelection(filename.Path().string());

  if (page == nullptr)
  {
    page = new wex::stc(text, 
      wex::stc_data(data).Window(wex::window_data().
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
  
wex::stc* frame::OpenFile(const wex::path& filename, const wex::stc_data& data)
{
  if ((data.Flags() & wex::stc_data::WIN_IS_PROJECT) && m_Projects == nullptr)
  {
    AddPaneProjects();
    GetManager().Update();
  }
  
  wex::notebook* notebook = ((data.Flags() & wex::stc_data::WIN_IS_PROJECT)
    ? m_Projects : m_Editors);
    
  wxASSERT(notebook != nullptr);
  
  wxWindow* page = notebook->SetSelection(filename.Path().string());

  if (data.Flags() & wex::stc_data::WIN_IS_PROJECT)
  {
    if (page == nullptr)
    {
      auto* project = new wex::listview_file(filename.Path().string(), wex::window_data().Parent(m_Projects));

      notebook->AddPage(
        project,
        filename.Path().string(),
        filename.GetName(),
        true,
        wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
          wex::get_iconid(filename)));
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

    if (filename == wex::vi_macros::GetFileName())
    {
      wex::vi_macros::SaveDocument();
    }
    
    auto* editor = (wex::stc*)page;

    if (page == nullptr)
    {
      editor = new wex::stc(filename, 
        wex::stc_data(data).
          Window(wex::window_data().Parent(m_Editors)).
          Flags(m_App->GetData().Flags(), wex::control_data::OR).
          Flags(wxConfigBase::Get()->ReadBool("HexMode", false) ? wex::stc_data::WIN_HEX: wex::stc_data::WIN_DEFAULT, wex::control_data::OR).
          Menu(m_App->GetDebug() ? wex::stc_data::MENU_DEBUG: wex::stc_data::MENU_NONE, wex::control_data::OR));
      
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
          wex::get_iconid(filename)));
          
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
        wex::tokenizer tkz(std::string((const char *)buffer->data(), buffer->length()), "\r\n");
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
      wex::stc_data(editor, data).Inject();
    }
    
    editor->SetFocus();
    
    return editor;
  }
  
  return (wex::stc*)page;
}

void frame::PrintEx(wex::ex* ex, const std::string& text)
{
  auto* page = (wex::stc*)m_Editors->SetSelection("Print");

  if (page == nullptr)
  {
    page = new wex::stc(text, wex::stc_data().
      Window(wex::window_data().Name("Print").Parent(m_Editors)));
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
  
wex::process* frame::Process(const std::string& command)
{
  m_Process->Execute(command, wex::process::EXEC_DEFAULT);
  return m_Process;
}

wex::stc* frame::RestorePage(const std::string& key)
{
  if (!m_SavedPage.empty() && IsOpen(m_SavedPage))
  {
    m_Editors->ChangeSelection(m_SavedPage);
    return (wex::stc *)m_Editors->GetPageByKey(m_SavedPage);
  }
  
  return nullptr;
}
  
bool frame::SaveCurrentPage(const std::string& key)
{
  m_SavedPage = m_Editors->GetCurrentPage();
  return true;
}

void frame::StatusBarClicked(const std::string& pane)
{
  if (pane == "PaneTheme")
  {
    if (wex::lexers::Get()->ShowThemeDialog(m_Editors))
    {
      m_Editors->ForEach<wex::stc>(wex::ID_ALL_STC_SET_LEXER_THEME);

      m_StatusBar->ShowField(
        "PaneLexer", 
        !wex::lexers::Get()->GetTheme().empty());
        
      StatusText(wex::lexers::Get()->GetTheme(), "PaneTheme");
    }
  }
  else if (pane == "PaneMacro")
  {
    if (auto* editor = GetSTC(); editor != nullptr) 
      editor->GetVi().GetMacros().Mode()->Transition("@", &editor->GetVi(), true);
  }
  else if (pane == "PaneVCS")
  {
    if (wex::vcs::GetCount() > 0)
    {
      auto* menu = new wex::menu;
      
      if (menu->AppendVCS())
      {
        PopupMenu(menu);
      }
      
      delete menu;
    }
  }
  else
  {
    decorated_frame::StatusBarClicked(pane);
  }
}

void frame::StatusBarClickedRight(const std::string& pane)
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
      if (wex::lexers::Get()->GetTheme().empty())
      {
        return;
      }
      
      match = "theme *name *= *\"" + wex::lexers::Get()->GetTheme() + "\"";
    }
    
    OpenFile(
      wex::lexers::Get()->GetFileName(), 
      wex::control_data().Find(match, wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX));
  }
  else if (pane == "PaneMacro")
  {
    if (wex::vi_macros::GetFileName().FileExists())
    {
      OpenFile(wex::vi_macros::GetFileName(),
        wex::control_data().Find(!GetStatusText(pane).empty() ? " name=\"" + GetStatusText(pane) + "\"":
          std::string()));
    }
  }
  else if (pane == "PaneVCS")
  {
    std::string match(GetStatusText(pane));

    if (auto* stc = GetSTC(); stc != nullptr)
    {
      const wex::vcs vcs({stc->GetFileName().Path().string()});
      match = vcs.GetEntry().GetName();
    }

    OpenFile(wex::menus::GetFileName(), wex::control_data().Find(match));
  }
  else
  {
    decorated_frame::StatusBarClickedRight(pane);
  }
}

void frame::SyncAll()
{
  m_Editors->ForEach<wex::stc>(wex::ID_ALL_STC_SYNC);
}

void frame::SyncCloseAll(wxWindowID id)
{
  decorated_frame::SyncCloseAll(id);
  
  if (IsClosing()) return;
  
  switch (id)
  {
  case wex::ID_NOTEBOOK_EDITORS:
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
  case wex::ID_NOTEBOOK_LISTS: ShowPane("OUTPUT", false); break;
  case wex::ID_NOTEBOOK_PROJECTS: ShowPane("PROJECTS", false); break;
  default: wxFAIL;
  }
}

editors::editors(const wex::window_data& data)
  : wex::notebook(data)
{
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxPostEvent(wxAuiNotebook::GetCurrentPage(), event);
    }, wex::ID_EDIT_VCS_LOWEST, wex::ID_EDIT_VCS_HIGHEST);

  Bind(wxEVT_AUINOTEBOOK_END_DRAG, [=](wxAuiNotebookEvent& event) {
    event.Skip();
    m_Split = true;
    });

  Bind(wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, [=](wxAuiNotebookEvent& event) {
    wex::menu menu;
    
    auto* split = new wex::menu;
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
    menu.Append(wex::ID_ALL_CLOSE, _("Close A&ll"));
    
    if (GetPageCount() > 2)
    {
      menu.Append(wex::ID_ALL_CLOSE_OTHERS, _("Close Others"));
    }

    if (auto* stc = wxDynamicCast(wxAuiNotebook::GetCurrentPage(), wex::stc);
      stc->GetFile().GetFileName().FileExists() && 
        wex::vcs::DirExists(stc->GetFile().GetFileName()))
    {
      menu.AppendSeparator();
      menu.AppendVCS(stc->GetFile().GetFileName());
    }
    
    PopupMenu(&menu);});
}
