////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of sample classes for wxExtension
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
#include <wx/textfile.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/printing.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/version.h>
#include "app.h"
#ifndef __WXMSW__
#include "app.xpm"
#endif

#include "../test/test-configitem.h"
#include "../test/test-item.h"

enum
{
  ID_DLG_CONFIG_ITEM,
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

wxIMPLEMENT_APP(wxExSampleApp);

bool wxExSampleApp::OnInit()
{
  SetAppName("wxex-sample");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExSampleFrame *frame = new wxExSampleFrame();
  frame->Show(true);
  
  wxLogStatus(
    "Locale: " + GetLocale().GetLocale() + " dir: " + GetCatalogDir());

  return true;
}

#if wxUSE_GRID
wxExSampleDir::wxExSampleDir(
  const wxString& fullpath, const wxString& findfiles, wxExGrid* grid)
  : wxExDir(fullpath.ToStdString(), findfiles.ToStdString())
  , m_Grid(grid)
{
}

bool wxExSampleDir::OnFile(const std::string& file)
{
  m_Grid->AppendRows(1);
  const auto no = m_Grid->GetNumberRows() - 1;
  m_Grid->SetCellValue(no, 0, wxString::Format("cell%d", no));
  m_Grid->SetCellValue(no, 1, file);

  // Let's make these cells readonly and colour them, so we can test
  // things like cutting and dropping is forbidden.
  m_Grid->SetReadOnly(no, 1);
  m_Grid->SetCellBackgroundColour(no, 1, *wxLIGHT_GREY);
  return true;
}
#endif

wxExSampleFrame::wxExSampleFrame()
  : wxExManagedFrame(nullptr, wxID_ANY, wxTheApp->GetAppDisplayName(), 4)
  , m_Process(new wxExProcess())
  , m_Notebook(new wxExNotebook(this, this,
      wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS))
  , m_STC(new wxExSTC(this))
  , m_Shell(new wxExShell(this, ">", "\n", true, 10))
  , m_STCLexers(new wxExSTC(this, wxExLexers::Get()->GetFileName()))
{
  wxExProcess::PrepareOutput(this);
  
  SetIcon(wxICON(app));

  wxExMenu* menuFile = new wxExMenu;
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

  wxExMenu *menuEdit = new wxExMenu();
  menuEdit->Append(wxID_UNDO);
  menuEdit->Append(wxID_REDO);
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_CUT);
  menuEdit->Append(wxID_COPY);
  menuEdit->Append(wxID_PASTE);
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_JUMP_TO);
  menuEdit->AppendSeparator();
  wxExMenu* menuFind = new wxExMenu();
  menuFind->Append(wxID_FIND);
  menuFind->Append(wxID_REPLACE);
  menuEdit->AppendSubMenu(menuFind, _("&Find And Replace"));
  
  wxExMenu* menuDialog = new wxExMenu;
  menuDialog->Append(ID_DLG_ITEM, wxExEllipsed("Item Dialog"));
  menuDialog->AppendSeparator();
  menuDialog->Append(ID_DLG_CONFIG_ITEM, wxExEllipsed("Config Dialog"));
  menuDialog->Append(ID_DLG_CONFIG_ITEM_COL, wxExEllipsed("Config Dialog Columns"));
  menuDialog->Append(ID_DLG_CONFIG_ITEM_READONLY, wxExEllipsed("Config Dialog Readonly"));
  menuDialog->AppendSeparator();
  menuDialog->Append(ID_DLG_LISTVIEW, wxExEllipsed("List Dialog"));
  menuDialog->AppendSeparator();
  menuDialog->Append(ID_DLG_STC_CONFIG, wxExEllipsed("STC Dialog"));
  menuDialog->Append(ID_DLG_STC_ENTRY, wxExEllipsed("STC Entry Dialog"));
  menuDialog->AppendSeparator();
  menuDialog->Append(ID_DLG_VCS, wxExEllipsed("VCS Dialog"));

  wxExMenu* menuSTC = new wxExMenu;
  menuSTC->Append(ID_STC_FLAGS, wxExEllipsed("Open Flag"));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_SPLIT, "Split");

  wxExMenu *menuView = new wxExMenu;
  AppendPanes(menuView);
  menuView->AppendSeparator();
  menuView->Append(ID_STATISTICS_SHOW, "Statistics");
  
  wxExMenu* menuHelp = new wxExMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, "&File");
  menubar->Append(menuEdit, "&Edit");
  menubar->Append(menuView, "&View");
  menubar->Append(menuDialog, "&Dialog");
  menubar->Append(menuSTC, "&STC");
  menubar->Append(menuHelp, "&Help");
  SetMenuBar(menubar);

#if wxUSE_GRID
  m_Grid = new wxExGrid(m_Notebook);
#endif
  m_ListView = new wxExListView(m_Notebook, wxExListView::LIST_NONE);

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

  m_Notebook->AddPage(m_STCLexers, wxExLexers::Get()->GetFileName().GetFullName());
  m_Notebook->AddPage(m_ListView, "wxExListView");

#if wxUSE_GRID
  m_Notebook->AddPage(m_Grid, "wxExGrid");
  m_Grid->CreateGrid(0, 0);
  m_Grid->AppendCols(2);
  wxExSampleDir dir(wxGetCwd(), "*.*", m_Grid);
  dir.FindFiles();
  m_Grid->AutoSizeColumns();
#endif

  m_ListView->SetSingleStyle(wxLC_REPORT);
  m_ListView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING));
  m_ListView->AppendColumn(wxExColumn("Number", wxExColumn::COL_INT));
  m_ListView->AppendColumn(wxExColumn("Float", wxExColumn::COL_FLOAT));
  m_ListView->AppendColumn(wxExColumn("Date", wxExColumn::COL_DATE));

  const int items = 50;

  for (auto i = 0; i < items; i++)
  {
    m_ListView->InsertItem(i, wxString::Format("item%d", i));
    m_ListView->SetItem(i, 1, std::to_string(i));
    m_ListView->SetItem(i, 2, wxString::Format("%f", (float)i / 2.0));
    m_ListView->SetItem(i, 3, wxDateTime::Now().FormatISOCombined(' '));

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

#if wxUSE_STATUSBAR
  SetupStatusBar(std::vector<wxExStatusBarPane>{
    wxExStatusBarPane(),
    wxExStatusBarPane("PaneFileType", 50, "File type"),
    wxExStatusBarPane("PaneInfo", 100, "Lines or items"),
    wxExStatusBarPane("PaneLexer", 60, "Lexer")});
#endif

  GetToolBar()->AddControls();
  GetOptionsToolBar()->AddControls();
  
  // The OnCommand keeps statistics.
  Bind(wxEVT_MENU, &wxExSampleFrame::OnCommand, this, wxID_COPY);
  Bind(wxEVT_MENU, &wxExSampleFrame::OnCommand, this, wxID_CUT);
  Bind(wxEVT_MENU, &wxExSampleFrame::OnCommand, this, wxID_EXECUTE);
  Bind(wxEVT_MENU, &wxExSampleFrame::OnCommand, this, wxID_JUMP_TO);
  Bind(wxEVT_MENU, &wxExSampleFrame::OnCommand, this, wxID_PASTE);
  Bind(wxEVT_MENU, &wxExSampleFrame::OnCommand, this, wxID_OPEN, wxID_SAVEAS);
  Bind(wxEVT_MENU, &wxExSampleFrame::OnCommand, this, wxID_UNDO, wxID_REDO);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wxExGetVersionInfo().GetVersionOnlyString());
    info.SetCopyright(wxExGetVersionInfo().GetCopyright());
    wxAboutBox(info);}, wxID_ABOUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->Print();}, wxID_PRINT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->PrintPreview();}, wxID_PREVIEW);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExPrinting::Get()->GetHtmlPrinter()->PageSetup();}, wxID_PRINT_SETUP);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const long val = wxGetNumberFromUser("Input columns:",
      wxEmptyString, _("Columns"), 1, 1, 100);
    if (val >= 0)
    {
      wxExItemDialog(this, TestConfigItems(0, val), 
        "Config Dialog Columns", 0, val).ShowModal();
    }}, ID_DLG_CONFIG_ITEM_COL);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExItemDialog* dlg = new wxExItemDialog(this, TestConfigItems(0, 1), 
      "Config Dialog",
      0, 1,
      wxAPPLY | wxCANCEL,
      wxID_ANY,
      wxDefaultPosition,
#ifdef __WXMSW__    
      wxSize(500, 500));
#else
      wxSize(600, 600));
#endif    
    //  dlg->ForceCheckBoxChecked("Group", "Checkboxes");
    dlg->Show();}, ID_DLG_CONFIG_ITEM);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExItemDialog(this, TestConfigItems(0, 1), "Config Dialog Readonly",
      0, 4, wxCANCEL).ShowModal();}, ID_DLG_CONFIG_ITEM_READONLY);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExItemDialog(this, TestItems(), "Options", 0, 1).ShowModal();}, ID_DLG_ITEM);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ListView->ConfigDialog(this);}, ID_DLG_LISTVIEW);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExSTC::ConfigDialog(this,
      "Editor Options",
      STC_CONFIG_MODELESS | STC_CONFIG_WITH_APPLY);}, ID_DLG_STC_CONFIG);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    std::string text;
    for (auto i = 0; i < 100; i++)
    {
      text += wxString::Format("Hello from line: %d\n", i);
    }
    wxExSTCEntryDialog(
      this,
      "Hello world",
      text,      
      "Greetings from " + wxTheApp->GetAppDisplayName()).ShowModal();
    }, ID_DLG_STC_ENTRY);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExVCS().ConfigDialog(this);}, ID_DLG_VCS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Shell->Prompt(
      "\nHello '" + event.GetString().ToStdString() + "' from the shell");}, ID_SHELL_COMMAND);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxFileDialog dlg(this, _("Open File"), "", "",
      "All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    const wxExVCS vcs(std::vector< std::string > {dlg.GetPath().ToStdString()});
    wxLogMessage(vcs.GetName().c_str());}, ID_SHOW_VCS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_Notebook->SetSelection("Statistics") == nullptr)
    {
      m_Notebook->AddPage(m_Statistics.Show(m_Notebook), "Statistics");
    }}, ID_STATISTICS_SHOW);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    long value = wxGetNumberFromUser("Input:",
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
  
  Bind(wxEVT_MENU, &wxExSampleFrame::OnCommand, this, ID_STC_SPLIT);
  
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(
      (GetListView() != nullptr && GetListView()->GetItemCount() > 0) ||
      (GetSTC() != nullptr && GetSTC()->GetLength() > 0));}, wxID_PRINT);
  
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(
      (GetListView() != nullptr && GetListView()->GetItemCount() > 0) ||
      (GetSTC() != nullptr && GetSTC()->GetLength() > 0));}, wxID_PREVIEW);
}

void wxExSampleFrame::OnCommand(wxCommandEvent& event)
{
  m_Statistics.Inc(std::to_string(event.GetId()));

  auto* editor = GetSTC();
  auto* grid = GetGrid();
  auto* listview = GetListView();

  switch (event.GetId())
  {
    case wxID_NEW:
      m_STC->GetFile().FileNew(wxExFileName());
      break;
    case wxID_OPEN:
      {
      wxExFileDialog dlg(this, &m_STC->GetFile());
      if (dlg.ShowModalIfChanged(true) == wxID_CANCEL) return;
      const auto start = std::chrono::system_clock::now();
      m_STC->Open(dlg.GetPath().ToStdString(), 0, std::string(), m_FlagsSTC);
      const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
      wxLogStatus(
        "wxExSTC::Open:%ld milliseconds, %d bytes", milli.count(), m_STC->GetTextLength());
      }
      break;
    case wxID_SAVE:
      m_STC->GetFile().FileSave();
  
      if (m_STC->GetFileName().GetFullPath() == 
          wxExLexers::Get()->GetFileName().GetFullPath())
      {
        wxExLexers::Get()->LoadDocument();
        wxLogMessage("File contains: %d lexers", wxExLexers::Get()->GetLexers().size());
          // As the lexer might have changed, update status bar field as well.
  #if wxUSE_STATUSBAR
        UpdateStatusBar(m_STC, "PaneLexer");
  #endif
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
        wxExSTC* stc = new wxExSTC(m_Notebook, editor->GetFileName());
        m_Notebook->AddPage(
          stc,
          wxString::Format("stc%d", stc->GetId()),
          m_STC->GetFileName().GetFullName());
        stc->SetDocPointer(m_STC->GetDocPointer());
      }
      break;
      
    default:
      wxFAIL;
      break;
    }
}

void wxExSampleFrame::OnCommandItemDialog(
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
    wxLogMessage(wxString::Format("button: %d checked: %d", event.GetId(), event.IsChecked()));
  }
  else
  {
    wxExManagedFrame::OnCommandItemDialog(dialogid, event);
  }
}
