////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <numeric>
#include <functional>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/numdlg.h>
#include <wex/defs.h>
#include <wex/filedlg.h>
#include <wex/itemdlg.h>
#include <wex/lexers.h>
#include <wex/printing.h>
#include <wex/stcdlg.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wex/version.h>
#include <easylogging++.h>
#include "app.h"
#ifndef __WXMSW__
#include "app.xpm"
#endif

#include "../test/test-configitem.h"
#include "../test/test-item.h"

enum
{
  ID_DLG_CONFIG_ITEM = wex::ID_EDIT_HIGHEST + 1,
  ID_DLG_CONFIG_ITEM_COL,
  ID_DLG_CONFIG_ITEM_READONLY,
  ID_DLG_ITEM,
  ID_DLG_LISTVIEW,
  ID_DLG_STC_CONFIG,
  ID_DLG_STC_ENTRY,
  ID_DLG_VCS,
  ID_RECENTFILE_MENU,
  ID_SHOW_VCS,
  ID_STATISTICS_SHOW,
  ID_STC_FLAGS,
  ID_STC_SPLIT,
};

wxIMPLEMENT_APP(sample_app);

bool sample_app::OnInit()
{
  SetAppName("wex-sample");

  if (!wex::app::OnInit())
  {
    return false;
  }

  sample_frame *frame = new sample_frame();
  frame->Show(true);
  
  wxLogStatus(
    "Locale: " + GetLocale().GetLocale() + " dir: " + GetCatalogDir());

  return true;
}

sample_dir::sample_dir(
  const std::string& fullpath, const std::string& findfiles, wex::grid* grid)
  : wex::dir(fullpath, findfiles)
  , m_Grid(grid)
{
}

bool sample_dir::OnFile(const wex::path& file)
{
  m_Grid->AppendRows(1);
  const auto no = m_Grid->GetNumberRows() - 1;
  m_Grid->SetCellValue(no, 0, wxString::Format("cell%d", no));
  m_Grid->SetCellValue(no, 1, file.Path().string());

  // Let's make these cells readonly and colour them, so we can test
  // things like cutting and dropping is forbidden.
  m_Grid->SetReadOnly(no, 1);
  m_Grid->SetCellBackgroundColour(no, 1, *wxLIGHT_GREY);
  return true;
}

sample_frame::sample_frame()
  : wex::managed_frame(4)
  , m_Process(new wex::process())
  , m_Notebook(new wex::notebook())
  , m_STC(new wex::stc())
  , m_Shell(new wex::shell(wex::stc_data(), ">", "\n"))
  , m_STCLexers(new wex::stc(wex::lexers::Get()->GetFileName()))
{
  wex::process::PrepareOutput(this);
  
  SetIcon(wxICON(app));

  wex::menu* menuFile = new wex::menu;
  menuFile->Append(wxID_OPEN);
  GetFileHistory().UseMenu(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(ID_SHOW_VCS, "Show VCS");
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXECUTE);
  menuFile->Append(wxID_STOP);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wex::menu *menuEdit = new wex::menu();
  menuEdit->Append(wxID_UNDO);
  menuEdit->Append(wxID_REDO);
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_CUT);
  menuEdit->Append(wxID_COPY);
  menuEdit->Append(wxID_PASTE);
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_JUMP_TO);
  menuEdit->AppendSeparator();
  wex::menu* menuFind = new wex::menu();
  menuFind->Append(wxID_FIND);
  menuFind->Append(wxID_REPLACE);
  menuEdit->AppendSubMenu(menuFind, _("&Find And Replace"));
  
  wex::menu* menuDialog = new wex::menu;
  menuDialog->Append(ID_DLG_ITEM, wex::ellipsed("Item Dialog"));
  menuDialog->AppendSeparator();
  menuDialog->Append(ID_DLG_CONFIG_ITEM, wex::ellipsed("Config Dialog"));
  menuDialog->Append(ID_DLG_CONFIG_ITEM_COL, wex::ellipsed("Config Dialog Columns"));
  menuDialog->Append(ID_DLG_CONFIG_ITEM_READONLY, wex::ellipsed("Config Dialog Readonly"));
  menuDialog->AppendSeparator();
  menuDialog->Append(ID_DLG_LISTVIEW, wex::ellipsed("List Dialog"));
  menuDialog->AppendSeparator();
  menuDialog->Append(ID_DLG_STC_CONFIG, wex::ellipsed("STC Dialog"));
  menuDialog->Append(ID_DLG_STC_ENTRY, wex::ellipsed("STC Entry Dialog"));
  menuDialog->AppendSeparator();
  menuDialog->Append(ID_DLG_VCS, wex::ellipsed("VCS Dialog"));

  wex::menu* menuSTC = new wex::menu;
  menuSTC->Append(ID_STC_FLAGS, wex::ellipsed("Open Flag"));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_SPLIT, "Split");

  wex::menu *menuView = new wex::menu;
  AppendPanes(menuView);
  menuView->AppendSeparator();
  menuView->Append(ID_STATISTICS_SHOW, "Statistics");
  
  wex::menu* menuHelp = new wex::menu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, "&File");
  menubar->Append(menuEdit, "&Edit");
  menubar->Append(menuView, "&View");
  menubar->Append(menuDialog, "&Dialog");
  menubar->Append(menuSTC, "&STC");
  menubar->Append(menuHelp, "&Help");
  SetMenuBar(menubar);

  m_Grid = new wex::grid(wex::window_data().Parent(m_Notebook));
  m_ListView = new wex::listview(wex::window_data().Parent(m_Notebook));

  GetManager().AddPane(m_Notebook, 
    wxAuiPaneInfo().CenterPane().MinSize(wxSize(250, 250)));
  GetManager().AddPane(m_STC, 
    wxAuiPaneInfo().Bottom().Caption("STC"));
  GetManager().AddPane(m_Shell, 
    wxAuiPaneInfo().Bottom().Caption("Shell").MinSize(wxSize(250, 250)));
  GetManager().AddPane(m_Process->GetShell(), wxAuiPaneInfo()
    .Bottom()
    .Name("PROCESS")
    .MinSize(250, 100)
    .Caption(_("Process")));

  GetManager().Update();

  m_Notebook->AddPage(m_STCLexers, wex::lexers::Get()->GetFileName().GetFullName());
  m_Notebook->AddPage(m_ListView, "wex::listview");

  m_Notebook->AddPage(m_Grid, "wex::grid");
  m_Grid->CreateGrid(0, 0);
  m_Grid->AppendCols(2);
  sample_dir dir(wex::path::Current(), "*.*", m_Grid);
  dir.FindFiles();
  m_Grid->AutoSizeColumns();

  m_ListView->AppendColumns({
    {"String", wex::column::STRING},
    {"Number", wex::column::INT},
    {"Float", wex::column::FLOAT},
    {"Date", wex::column::DATE}});

  const int items = 50;

  for (auto i = 0; i < items; i++)
  {
    m_ListView->InsertItem({
      "item " + std::to_string(i),
      std::to_string(i),
      std::to_string((float)i / 2.0),
      wxDateTime::Now().Format("%c").ToStdString()});

    // Set some images.
    if      (i == 0) m_ListView->SetItemImage(i, wxART_CDROM);
    else if (i == 1) m_ListView->SetItemImage(i, wxART_REMOVABLE);
    else if (i == 2) m_ListView->SetItemImage(i, wxART_FOLDER);
    else if (i == 3) m_ListView->SetItemImage(i, wxART_FOLDER_OPEN);
    else if (i == 4) m_ListView->SetItemImage(i, wxART_GO_DIR_UP);
    else if (i == 5) m_ListView->SetItemImage(i, wxART_EXECUTABLE_FILE);
    else if (i == 6) m_ListView->SetItemImage(i, wxART_NORMAL_FILE);
    else             m_ListView->SetItemImage(i, wxART_TICK_MARK);
  }

  SetupStatusBar({
    {"PaneFileType", 50, "File type"},
    {"PaneInfo", 100, "Lines or items"},
    {"PaneLexer", 60}});

  GetToolBar()->AddControls();
  GetOptionsToolBar()->AddControls();
  
  // The OnCommand keeps statistics.
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_COPY);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_CUT);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_EXECUTE);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_JUMP_TO);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_PASTE);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_OPEN, wxID_SAVEAS);
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, wxID_UNDO, wxID_REDO);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wex::get_version_info().Get());
    info.SetCopyright(wex::get_version_info().Copyright());
    wxAboutBox(info);}, wxID_ABOUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->Print();}, wxID_PRINT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->PrintPreview();}, wxID_PREVIEW);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::printing::Get()->GetHtmlPrinter()->PageSetup();}, wxID_PRINT_SETUP);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const long val = wxGetNumberFromUser("Input columns:",
      wxEmptyString, _("Columns"), 1, 1, 100);
    if (val >= 0)
    {
      wex::item_dialog(TestConfigItems(0, val), 
        wex::window_data().Title("Config Dialog Columns"), 0, val).ShowModal();
    }}, ID_DLG_CONFIG_ITEM_COL);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::item_dialog* dlg = new wex::item_dialog(TestConfigItems(0, 1), 
      wex::window_data().
        Title("Config Dialog").
        Button(wxAPPLY | wxCANCEL).
#ifdef __WXMSW__    
        Size(wxSize(500, 500)));
#else
        Size(wxSize(600, 600)));
#endif    
    //  dlg->ForceCheckBoxChecked("Group", "Checkboxes");
    dlg->Show();}, ID_DLG_CONFIG_ITEM);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::item_dialog(TestConfigItems(0, 1), 
      wex::window_data().
        Button(wxCANCEL).
        Title("Config Dialog Readonly"), 0, 4).ShowModal();}, ID_DLG_CONFIG_ITEM_READONLY);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::item_dialog(TestItems()).ShowModal();}, ID_DLG_ITEM);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->ConfigDialog();}, ID_DLG_LISTVIEW);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::stc::ConfigDialog(
      wex::window_data().Button(wxAPPLY | wxCANCEL));}, ID_DLG_STC_CONFIG);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    std::string text;
    for (auto i = 0; i < 100; i++)
    {
      text += wxString::Format("Hello from line: %d\n", i);
    }
    wex::stc_entry_dialog(
      text,      
      "Greetings from " + std::string(wxTheApp->GetAppDisplayName()),
      wex::window_data().Title("Hello world")).ShowModal();
    }, ID_DLG_STC_ENTRY);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::vcs().ConfigDialog();}, ID_DLG_VCS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Shell->Prompt(
      "\nHello '" + event.GetString().ToStdString() + "' from the shell");}, wex::ID_SHELL_COMMAND);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxFileDialog dlg(this, _("Open File"), "", "",
      "All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    const wex::vcs vcs(std::vector< wex::path > {dlg.GetPath().ToStdString()});
    wex::stc_entry_dialog(vcs.GetName()).ShowModal();}, ID_SHOW_VCS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_Notebook->SetSelection("Statistics") == nullptr)
    {
      m_Notebook->AddPage(m_Statistics.Show(m_Notebook), "Statistics");
    }}, ID_STATISTICS_SHOW);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const long value = wxGetNumberFromUser("Input:",
      wxEmptyString, "STC Open Flag",
      m_FlagsSTC,
      0,
      0xFFFF);
    if (value != -1)
    {
      m_FlagsSTC = value;
    }}, ID_STC_FLAGS);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Process->Execute();}, wxID_EXECUTE);
  
  Bind(wxEVT_MENU, &sample_frame::OnCommand, this, ID_STC_SPLIT);
  
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(
      (GetListView() != nullptr && GetListView()->GetItemCount() > 0) ||
      (GetSTC() != nullptr && GetSTC()->GetLength() > 0));}, wxID_PRINT);
  
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(
      (GetListView() != nullptr && GetListView()->GetItemCount() > 0) ||
      (GetSTC() != nullptr && GetSTC()->GetLength() > 0));}, wxID_PREVIEW);
}

void sample_frame::OnCommand(wxCommandEvent& event)
{
  m_Statistics.Inc(std::to_string(event.GetId()));

  auto* editor = GetSTC();
  auto* grid = GetGrid();
  auto* listview = GetListView();

  switch (event.GetId())
  {
    case wxID_NEW:
      m_STC->GetFile().FileNew(wex::path());
      break;
    case wxID_OPEN:
      {
      wex::file_dialog dlg(&m_STC->GetFile());
      if (dlg.ShowModalIfChanged(true) == wxID_CANCEL) return;
      const auto start = std::chrono::system_clock::now();
      m_STC->Open(dlg.GetPath().ToStdString(), 
        wex::stc_data().Flags((wex::stc_data::window_flags)m_FlagsSTC));
      const auto milli = std::chrono::duration_cast
        <std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
      wxLogStatus(
        "wex::stc::Open:%ld milliseconds, %d bytes", 
        milli.count(), m_STC->GetTextLength());
      }
      break;
    case wxID_SAVE:
      m_STC->GetFile().FileSave();
  
      if (m_STC->GetFileName().Path() == wex::lexers::Get()->GetFileName().Path())
      {
        wex::lexers::Get()->LoadDocument();
        VLOG(9) << "File contains: " << wex::lexers::Get()->get().size() << " lexers";
          // As the lexer might have changed, update status bar field as well.
        UpdateStatusBar(m_STC, "PaneLexer");
      }
      break;
  
    case wxID_COPY:
    case wxID_CUT:
    case wxID_JUMP_TO:
    case wxID_PASTE:
    case wxID_REDO:
    case wxID_UNDO:
      if (editor != nullptr)
      {
        wxPostEvent(editor, event);
      }
      else if (grid != nullptr)
      {
        wxPostEvent(grid, event);
      }
      else if (listview != nullptr)
      {
        wxPostEvent(listview, event);
      }
    break;

    case ID_STC_SPLIT:
      if (editor != nullptr)
      {
        wex::stc* stc = new wex::stc(editor->GetFileName(), 
          wex::stc_data().Window(wex::window_data().Parent(m_Notebook)));
        m_Notebook->AddPage(
          stc,
          "stc" + std::to_string(stc->GetId()),
          m_STC->GetFileName().GetFullName());
        stc->SetDocPointer(m_STC->GetDocPointer());
      }
      break;
      
    default:
      wxFAIL;
      break;
    }
}

void sample_frame::OnCommandItemDialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  if (dialogid == wxID_PREFERENCES)
  {
    if (event.GetId() != wxID_CANCEL)
    {
      m_STC->ConfigGet();
      m_STCLexers->ConfigGet();
    }
  }
  else if (event.GetId() >= 1000 && event.GetId() < 1050)
  {
    VLOG(9) << "button: " << event.GetId() << " checked: " << event.IsChecked();
  }
  else
  {
    wex::managed_frame::OnCommandItemDialog(dialogid, event);
  }
}
