////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of sample classes for wxExtension
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <numeric>
#include <functional>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/numdlg.h>
#include <wx/html/htmlwin.h>
#include <wx/textfile.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/printing.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/version.h>
#include "app.h"
#ifndef __WXMSW__
#include "app.xpm"
#endif

enum
{
  ID_FIRST = 15000,
  ID_CONFIG_DLG,
  ID_CONFIG_DLG_1_COL,
  ID_CONFIG_DLG_4_COL,
  ID_CONFIG_DLG_READONLY,
  ID_SHOW_VCS,
  ID_PROCESS_SELECT,
  ID_PROCESS_RUN,
  ID_STATISTICS_SHOW,
  ID_STC_CONFIG_DLG,
  ID_STC_ENTRY_DLG,
  ID_STC_FLAGS,
  ID_STC_SPLIT,
  ID_LAST 
};

void myHtmlCreate(wxWindow* user, wxWindow* parent, bool readonly)
{
  ((wxHtmlWindow *)user)->Create(parent, 100);
}

void myTextCreate(wxWindow* user, wxWindow* parent, bool readonly)
{
  ((wxTextCtrl *)user)->Create(parent, 100);
}

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
  : wxExDir(fullpath, findfiles)
  , m_Grid(grid)
{
}

void wxExSampleDir::OnFile(const wxString& file)
{
  m_Grid->AppendRows(1);
  const auto no = m_Grid->GetNumberRows() - 1;
  m_Grid->SetCellValue(no, 0, wxString::Format("cell%d", no));
  m_Grid->SetCellValue(no, 1, file);

  // Let's make these cells readonly and colour them, so we can test
  // things like cutting and dropping is forbidden.
  m_Grid->SetReadOnly(no, 1);
  m_Grid->SetCellBackgroundColour(no, 1, *wxLIGHT_GREY);
}
#endif

BEGIN_EVENT_TABLE(wxExSampleFrame, wxExManagedFrame)
  EVT_MENU(wxID_JUMP_TO, wxExSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_OPEN, wxID_PREFERENCES, wxExSampleFrame::OnCommand)
  EVT_MENU_RANGE(ID_FIRST, ID_LAST, wxExSampleFrame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND, wxExSampleFrame::OnCommand)
  EVT_UPDATE_UI(wxID_PRINT, wxExSampleFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_PREVIEW, wxExSampleFrame::OnUpdateUI)
END_EVENT_TABLE()

wxExSampleFrame::wxExSampleFrame()
  : wxExManagedFrame(NULL, wxID_ANY, wxTheApp->GetAppDisplayName())
  , m_FlagsSTC(0)
{
  SetIcon(wxICON(app));

  wxExMenu* menuFile = new wxExMenu;
  menuFile->Append(wxID_OPEN);
  menuFile->AppendSeparator();
  menuFile->Append(ID_SHOW_VCS, "Show VCS");
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(ID_PROCESS_SELECT, "Select Process");
  menuFile->Append(ID_PROCESS_RUN, "Run Process");
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
  
  wxExMenu* menuConfig = new wxExMenu;
  menuConfig->Append(ID_CONFIG_DLG, wxExEllipsed("Config Dialog"));
  menuConfig->Append(ID_CONFIG_DLG_1_COL, wxExEllipsed("Config Dialog 1 Col"));
  menuConfig->Append(ID_CONFIG_DLG_4_COL, wxExEllipsed("Config Dialog 4 Col"));
  menuConfig->Append(
    ID_CONFIG_DLG_READONLY, 
    wxExEllipsed("Config Dialog Readonly"));

  wxExMenu* menuSTC = new wxExMenu;
  menuSTC->Append(ID_STC_FLAGS, wxExEllipsed("Open Flag"));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_CONFIG_DLG, wxExEllipsed("Config Dialog"));
  menuSTC->Append(ID_STC_ENTRY_DLG, wxExEllipsed("Entry Dialog"));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_SPLIT, "Split");

  wxExMenu *menuView = new wxExMenu;
  menuView->AppendBars();
  menuView->AppendSeparator();
  menuView->Append(ID_STATISTICS_SHOW, "Statistics");
  
  wxExMenu* menuHelp = new wxExMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, "&File");
  menubar->Append(menuEdit, "&Edit");
  menubar->Append(menuView, "&View");
  menubar->Append(menuSTC, "&STC");
  menubar->Append(menuConfig, "&Config");
  menubar->Append(menuHelp, "&Help");
  SetMenuBar(menubar);

  m_Notebook = new wxExNotebook(
    this, 
    NULL,
    wxID_ANY,
    wxDefaultPosition,
    wxDefaultSize,
    wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS);
#if wxUSE_GRID
  m_Grid = new wxExGrid(m_Notebook);
#endif
  m_ListView = new wxExListView(m_Notebook);
  m_STC = new wxExSTC(this);
  m_STCShell = new wxExSTCShell(this, ">", wxTextFile::GetEOL(), true, 10);

  GetManager().AddPane(m_Notebook, 
    wxAuiPaneInfo().CenterPane().MinSize(wxSize(250, 250)));
  GetManager().AddPane(m_STC, 
    wxAuiPaneInfo().Bottom().Caption("STC"));
  GetManager().AddPane(m_STCShell, 
    wxAuiPaneInfo().Bottom().Caption("Shell").MinSize(wxSize(250, 250)));

  GetManager().Update();

  m_STCLexers = new wxExSTC(this, wxExLexers::Get()->GetFileName());
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
    m_ListView->SetItem(i, 1, wxString::Format("%d", i));
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
  std::vector<wxExStatusBarPane> panes;
  panes.push_back(wxExStatusBarPane());
  panes.push_back(wxExStatusBarPane("PaneFileType", 50, "File type"));
  panes.push_back(wxExStatusBarPane("PaneInfo", 100, "Lines or items"));
  panes.push_back(wxExStatusBarPane("PaneLexer", 60, "Lexer"));
  SetupStatusBar(panes);
#endif

  if (wxExLexers::Get()->GetCount() > 0)
  {
    if (!wxExLexers::Get()->GetDefaultStyle().IsOk())
    {
      wxMessageBox("lexers default style not ok");
    }
    
    if (!wxExLexers::Get()->GetDefaultStyle().ContainsDefaultStyle())
    {
      wxMessageBox("lexers default style does not contain default style");
    }
  }
}

void wxExSampleFrame::OnCommand(wxCommandEvent& event)
{
  m_Statistics.Inc(wxString::Format("%d", event.GetId()));

  auto* editor = GetSTC();
  auto* grid = GetGrid();
  auto* listview = GetListView();

  if (
    (event.GetId() == wxID_UNDO ||
     event.GetId() == wxID_REDO ||
     event.GetId() == wxID_DELETE ||
     event.GetId() == wxID_SELECTALL ||
     event.GetId() == wxID_JUMP_TO) ||
    (event.GetId() >= wxID_CUT && event.GetId() <= wxID_CLEAR))
  {
    if (editor != NULL)
    {
      wxPostEvent(editor, event);
    }
    else if (grid != NULL)
    {
      wxPostEvent(grid, event);
    }
    else if (listview != NULL)
    {
      wxPostEvent(listview, event);
    }
  }
  else
  {
    switch (event.GetId())
    {
    case wxID_ABOUT:
      {
      wxAboutDialogInfo info;
      info.SetIcon(GetIcon());
      info.SetVersion(wxExGetVersionInfo().GetVersionOnlyString());
      info.SetCopyright(wxExGetVersionInfo().GetCopyright());
      wxAboutBox(info);
      }
      break;
    case wxID_EXIT: Close(true); break;
    case wxID_NEW:
      m_STC->GetFile().FileNew(wxExFileName());
      break;
    case wxID_OPEN:
      {
      wxExFileDialog dlg(this, &m_STC->GetFile());
      if (dlg.ShowModalIfChanged(true) == wxID_CANCEL) return;
  
      wxStopWatch sw;
      
      m_STC->Open(dlg.GetPath(), 0, wxEmptyString, m_FlagsSTC);
  
      const auto stop = sw.Time();
      wxLogStatus(
        "wxExSTC::Open:%ld milliseconds, %d bytes", stop, m_STC->GetTextLength());
      }
      break;
  
    case wxID_PREVIEW: m_ListView->PrintPreview(); break;
    case wxID_PRINT: m_ListView->Print(); break;
    case wxID_PRINT_SETUP: wxExPrinting::Get()->GetHtmlPrinter()->PageSetup(); break;
  
    case wxID_SAVE:
      m_STC->GetFile().FileSave();
  
      if (m_STC->GetFileName().GetFullPath() == 
          wxExLexers::Get()->GetFileName().GetFullPath())
      {
        wxExLexers::Get()->LoadDocument();
        wxLogMessage("File contains: %d lexers", wxExLexers::Get()->GetCount());
          // As the lexer might have changed, update status bar field as well.
  #if wxUSE_STATUSBAR
        UpdateStatusBar(m_STC, "PaneLexer");
  #endif
      }
      break;
  
    case ID_CONFIG_DLG: ShowConfigItems(); break;
    
    case ID_CONFIG_DLG_1_COL:
      {
      std::vector<wxExConfigItem> v;
  
      for (int sl = 1; sl <= 3; sl++)
      {
        v.push_back(wxExConfigItem(
          wxString::Format("Slider%d", sl),
          1,
          10,
          wxEmptyString,
          CONFIG_SLIDER,
          wxSL_HORIZONTAL,
          1,
          1));
      }
      
      v.push_back(wxExConfigItem("Group Checkbox1", CONFIG_CHECKBOX));
      v.push_back(wxExConfigItem(
        "STC cpp", 
        "cpp",
        wxEmptyString,
        0,
        CONFIG_STC));
    
      v.push_back(wxExConfigItem(
        "STC pascal", 
        "pascal",
        wxEmptyString,
        0,
        CONFIG_STC));
    
      wxExConfigItem item(
        "STC lisp", 
        "lisp",
        wxEmptyString,
        0,
        CONFIG_STC);
        
      item.SetRowGrowable(false);
      
      v.push_back(item);
    
      wxExConfigDialog* dlg = new wxExConfigDialog(
        this,
        v,
        "Config Dialog 1 Col",
        0,
        1);
  
      dlg->Show();
      }
      break;
    
    case ID_CONFIG_DLG_4_COL:
      {
      std::vector<wxExConfigItem> v;
  
      for (int sl = 1; sl <= 6; sl++)
      {
        v.push_back(wxExConfigItem(
          wxString::Format("Slider%d", sl),
          1,
          10,
          wxEmptyString,
          CONFIG_SLIDER));
      }
      
      v.push_back(wxExConfigItem("Group Checkbox1", CONFIG_CHECKBOX));
      v.push_back(wxExConfigItem("Group Checkbox2", CONFIG_CHECKBOX));
      v.push_back(wxExConfigItem("Group Checkbox3", CONFIG_CHECKBOX));
    
      wxExConfigDialog* dlg = new wxExConfigDialog(
        this,
        v,
        "Config Dialog 4 Col",
        0,
        4);
  
      dlg->Show();
      }
      break;
    
    case ID_CONFIG_DLG_READONLY:
      {
      std::vector<wxExConfigItem> v;
  
      v.push_back(wxExConfigItem());
      v.push_back(wxExConfigItem());
      v.push_back(wxExConfigItem("File Picker", CONFIG_FILEPICKERCTRL));
      v.push_back(wxExConfigItem("File Picker", CONFIG_FILEPICKERCTRL));
      v.push_back(wxExConfigItem("File Picker", CONFIG_FILEPICKERCTRL));
      v.push_back(wxExConfigItem("File Picker", CONFIG_FILEPICKERCTRL));
      v.push_back(wxExConfigItem("File Picker", CONFIG_FILEPICKERCTRL));
  
      for (int j = 1; j <= 10; j++)
      {
        v.push_back(wxExConfigItem(wxString::Format("Integer%d", j), CONFIG_INT));
      }
  
      wxExConfigDialog* dlg = new wxExConfigDialog(
        this,
        v,
        "Config Dialog Readonly",
        0,
        4,
        wxCANCEL);
  
        dlg->Show();
      }
      break;
      
    case ID_PROCESS_SELECT:
      wxExProcess::ConfigDialog(this);
      break;
  
    case ID_PROCESS_RUN:
      m_Process.Execute(wxEmptyString);
      break;
      
    case ID_SHELL_COMMAND:
        m_STCShell->Prompt("\nHello '" + event.GetString() + "' from the shell");
      break;
      
    case ID_SHOW_VCS:
      {
      wxFileDialog openFileDialog(this, _("Open File"), "", "",
        "All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  
      if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...
          
      std::vector< wxString > v;
      v.push_back(openFileDialog.GetPath());
      wxExVCS vcs(v);
      wxLogMessage(vcs.GetName());
      }
      break;
  
    case ID_STATISTICS_SHOW:
      if (m_Notebook->SetSelection("Statistics") == NULL)
      {
        m_Notebook->AddPage(m_Statistics.Show(m_Notebook), "Statistics");
      }
      break;
  
    case ID_STC_CONFIG_DLG:
      wxExSTC::ConfigDialog(
        this,
        "Editor Options",
        wxExSTC::STC_CONFIG_MODELESS | wxExSTC::STC_CONFIG_WITH_APPLY);
      break;
      
    case ID_STC_ENTRY_DLG:
      {
      wxString text;
      
      for (auto i = 0; i < 100; i++)
      {
        text += wxString::Format("Hello from line: %d\n", i);
      }
      
      wxExSTCEntryDialog dlg(
        this,
        "Hello world",
        text,      
        "Greetings from " + wxTheApp->GetAppDisplayName());
        
        dlg.ShowModal();
      }
      break;
      
    case ID_STC_FLAGS:
      {
      long value = wxGetNumberFromUser(
        "Input:",
        wxEmptyString,
        "STC Open Flag",
        m_FlagsSTC,
        0,
        0xFFFF);
  
      if (value != -1)
      {
        m_FlagsSTC = value;
      }
      }
      break;
      
    case ID_STC_SPLIT:
      {
      wxExSTC* stc = new wxExSTC(*m_STC);
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
}

void wxExSampleFrame::OnCommandConfigDialog(
  wxWindowID dialogid,
  int commandid)
{
  if (dialogid == wxID_PREFERENCES)
  {
    if (commandid != wxID_CANCEL)
    {
      m_STC->ConfigGet();
      m_STCLexers->ConfigGet();
    }
  }
  else if (commandid > 1000 && commandid < 1020)
  {
    wxLogMessage(wxString::Format("hello%d", commandid));
  }
  else
  {
    wxExManagedFrame::OnCommandConfigDialog(dialogid, commandid);
  }
}

void wxExSampleFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
    case wxID_PREVIEW:
    case wxID_PRINT:
      event.Enable(
        (GetListView() != NULL && GetListView()->GetItemCount() > 0) ||
        (GetSTC() != NULL && GetSTC()->GetLength() > 0));
      break;
  }
}

void wxExSampleFrame::ShowConfigItems()
{
  std::vector<wxExConfigItem> v;

  // CONFIG_BUTTON
  for (int b = 1; b <= 4; b++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("<span size='x-large' color='blue'>Big</span> <b>bold</b> button %d", b),
      CONFIG_BUTTON,
      "Buttons",
      false,
      1000 + b));
  }

  // CONFIG_CHECKBOX
  for (int h = 1; h <= 4; h++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Checkbox%d", h), 
      CONFIG_CHECKBOX, 
      "Checkboxes"));
  }

  v.push_back(wxExConfigItem(
    "Group Checkbox1",
    CONFIG_CHECKBOX, 
    "Checkboxes"));

  v.push_back(wxExConfigItem(
    "Group Checkbox2",
    CONFIG_CHECKBOX, 
    "Checkboxes"));

  // CONFIG_CHECKLISTBOX
  std::map<long, const wxString> clb;
  clb.insert(std::make_pair(0, "Bit One"));
  clb.insert(std::make_pair(1, "Bit Two"));
  clb.insert(std::make_pair(2, "Bit Three"));
  clb.insert(std::make_pair(4, "Bit Four"));
  v.push_back(wxExConfigItem(
    "Bin Choices", 
    clb, 
    false, 
    "Checkbox lists"));

  // CONFIG_CHECKLISTBOX_NONAME
  std::set<wxString> bchoices;
  bchoices.insert("This");
  bchoices.insert("Or");
  bchoices.insert("Other");
  v.push_back(wxExConfigItem(
    bchoices, 
    "Checkbox lists"));

  // CONFIG_COLOUR
  for (int i = 1; i <= 5; i++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Colour%d", i), 
      CONFIG_COLOUR, 
      "Colours"));
  }

  // CONFIG_COMBOBOX
  for (int m = 1; m <= 5; m++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Combobox%d", m), 
      CONFIG_COMBOBOX, 
      "Comboboxes"));
  }

  // CONFIG_COMBOBOX without a name
  v.push_back(wxExConfigItem(
    "Combobox No Name",
    CONFIG_COMBOBOX, 
    "Comboboxes",
    false,
    wxID_ANY,
    25,
    false));

  // CONFIG_COMBOBOXDIR
  v.push_back(wxExConfigItem(
    "Combobox Dir Required",
    CONFIG_COMBOBOXDIR, 
    "Comboboxes",
    true,
    1005));

  // CONFIG_COMBOBOXDIR
  v.push_back(wxExConfigItem(
    "Combobox Dir", 
    CONFIG_COMBOBOXDIR, 
    "Comboboxes",
    false,
    1006));

  // CONFIG_COMMAND_LINK_BUTTON
  for (int l = 1; l <= 4; l++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Command Link Button%d\tThis text describes what the button does", l),
      CONFIG_COMMAND_LINK_BUTTON,
      "Command Link Buttons",
      false,
      1007 + l));
  }

  // CONFIG_EMPTY
  v.push_back(wxExConfigItem(10, "Pickers", CONFIG_EMPTY));

  // CONFIG_DIRPICKERCTRL
  v.push_back(wxExConfigItem(
    "Dir Picker", 
    CONFIG_DIRPICKERCTRL, 
    "Pickers"));

  // CONFIG_FILEPICKERCTRL
  v.push_back(wxExConfigItem(
    "File Picker", 
    CONFIG_FILEPICKERCTRL, 
    "Pickers"));

  // CONFIG_FONTPICKERCTRL
  v.push_back(wxExConfigItem(
    "Font Picker", 
    CONFIG_FONTPICKERCTRL, 
    "Pickers"));

  // CONFIG_FLOAT
  v.push_back(wxExConfigItem(
    "Float", 
    CONFIG_FLOAT, 
    "Floats", 
    true));
      
  // CONFIG_HYPERLINKCTRL
  v.push_back(wxExConfigItem(
    "Hyper Link 1",
    "www.wxwidgets.org",
    "Hyperlinks",
    0,
    CONFIG_HYPERLINKCTRL));

  v.push_back(wxExConfigItem(
    "Hyper Link 2",
    "www.scintilla.org",
    "Hyperlinks",
    0,
    CONFIG_HYPERLINKCTRL));

  // CONFIG_INT
  for (int j = 1; j <= 5; j++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Integer%d", j), 
      CONFIG_INT, 
      "Integers", 
      true));
  }

  // CONFIG_LISTVIEW_FOLDER
  v.push_back(wxExConfigItem(
    "ListView",
    CONFIG_LISTVIEW_FOLDER,
    "ListView"));

  // CONFIG_RADIOBOX
  std::map<long, const wxString> echoices;
  echoices.insert(std::make_pair(0, "Zero"));
  echoices.insert(std::make_pair(1, "One"));
  echoices.insert(std::make_pair(2, "Two"));
  v.push_back(wxExConfigItem(
    "Radio Box", 
    echoices, 
    true, 
    "Radioboxes"));

  // CONFIG_SLIDER
  const int start = 1;
  for (int sl = start + 1; sl <= start + 3; sl++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Slider%d", sl),
      start,
      sl,
      "Spin controls",
      CONFIG_SLIDER));
  }

  // CONFIG_SPINCTRL
  for (int s = 1; s <= 2; s++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Spin Control%d", s), 
      1, 
      s, 
      "Spin controls"));
  }

  // CONFIG_SPINCTRL_DOUBLE
  for (int sd = 1; sd <= 2; sd++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Spin Control Double%d", sd), 
      1.0,
      (double)sd, 
      "Spin controls",
      CONFIG_SPINCTRL_DOUBLE,
      wxSL_HORIZONTAL,
      0.01));
  }

  // CONFIG_SPINCTRL_HEX
  for (int s = 1; s <= 2; s++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Spin Control Hex%d", s), 
      0, 
      255, 
      "Spin controls",
      CONFIG_SPINCTRL_HEX));
  }

  for (int st = 1; st <= 5; st++)
  {
    // CONFIG_STATICTEXT
    v.push_back(wxExConfigItem(
      wxString::Format("Static Text%d", st),
      "this is my static text",
      "Static Text",
      0,
      CONFIG_STATICTEXT));
  }

  // CONFIG_STATICLINE (horizontal)
  v.push_back(wxExConfigItem(wxLI_HORIZONTAL, "Static Line"));

  // CONFIG_STATICLINE (vertical)
  v.push_back(wxExConfigItem(wxLI_VERTICAL, "Static Line"));

  // CONFIG_STC
  v.push_back(wxExConfigItem(
    "STC", 
    "cpp",
    "STC",
    0,
    CONFIG_STC));

  // CONFIG_STRING
  for (int l = 1; l <= 5; l++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("String%d", l), 
      wxEmptyString,
      "Strings"));
  }
  
  wxExConfigItem ci(
    "String Validator", 
    wxEmptyString,
    "Strings");
  wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
  validator.SetCharIncludes("0123");
  ci.SetValidator(&validator);
  v.push_back(ci);
      
  v.push_back(wxExConfigItem(
    "String Multiline", 
    wxEmptyString,
    "Strings",
    wxTE_MULTILINE));

  // CONFIG_TOGGLEBUTTON
  for (int tb = 1; tb <= 4; tb++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Toggle Button%d", tb),
      CONFIG_TOGGLEBUTTON,
      "Toggle buttons"));
  }

  /// CONFIG_USER
  v.push_back(wxExConfigItem(
    "HTML Control", 
    new wxHtmlWindow(),
    myHtmlCreate,
    NULL,
    "User Controls"));

  v.push_back(wxExConfigItem(
    "Text Control", 
    new wxTextCtrl(),
    myTextCreate,
    NULL,
    "User Controls"));
    
  wxExConfigDialog* dlg = new wxExConfigDialog(
    this,
    v,
    "Config Dialog",
    0,
    1,
    wxAPPLY | wxCANCEL,
    wxID_ANY,
    wxExConfigDialog::CONFIG_LISTBOOK,
    wxDefaultPosition,
#ifdef __WXMSW__    
    wxSize(500, 500));
#else
    wxSize(600, 600));
#endif    
  
  dlg->ForceCheckBoxChecked("Group", "Checkboxes");
  dlg->Show();
}
