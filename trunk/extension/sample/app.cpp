/******************************************************************************\
* File:          app.cpp
* Purpose:       Implementation of sample classes for wxExtension
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/aboutdlg.h>
#include <wx/numdlg.h>
#include <wx/textfile.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/printing.h>
#include <wx/extension/renderer.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include "app.h"
#ifndef __WXMSW__
#include "app.xpm"
#endif

enum
{
  ID_FIRST = 15000,
  ID_CONFIG_DLG,
  ID_LEXER_PROPERTIES,
  ID_CONFIG_DLG_READONLY,
  ID_STATISTICS_SHOW,
  ID_STC_CONFIG_DLG,
  ID_STC_ENTRY_DLG,
  ID_STC_FLAGS,
  ID_STC_GOTO,
  ID_STC_SPLIT,
  ID_STC_LEXER,
  ID_LAST,
};

BEGIN_EVENT_TABLE(wxExSampleFrame, wxExFrame)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_OPEN, wxID_PREFERENCES, wxExSampleFrame::OnCommand)
  EVT_MENU_RANGE(ID_FIRST, ID_LAST, wxExSampleFrame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND, wxExSampleFrame::OnCommand)
END_EVENT_TABLE()

IMPLEMENT_APP(wxExSampleApp)

bool wxExSampleApp::OnInit()
{
  SetAppName("wxExSample");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExLog::Get()->SetLogging();

  wxExSampleFrame *frame = new wxExSampleFrame();
  frame->Show(true);
  frame->StatusText("Locale: " + GetLocale().GetLocale());

  SetTopWindow(frame);

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
  const int no = m_Grid->GetNumberRows() - 1;
  m_Grid->SetCellValue(no, 0, wxString::Format("cell%d", no));
  m_Grid->SetCellValue(no, 1, file);

  wxExRenderer* renderer = new wxExRenderer(
    wxExRenderer::CELL_CROSS, *wxGREEN_PEN, *wxRED_PEN);
  m_Grid->SetCellRenderer(no, 0, renderer);

  // Let's make these cells readonly and colour them, so we can test
  // things like cutting and dropping is forbidden.
  m_Grid->SetReadOnly(no, 1);
  m_Grid->SetCellBackgroundColour(no, 1, *wxLIGHT_GREY);
}
#endif

wxExSampleFrame::wxExSampleFrame()
  : wxExManagedFrame(NULL, wxID_ANY, wxTheApp->GetAppName())
  , m_FlagsSTC(0)
{
  SetIcon(wxICON(app));

  wxExMenu* menuFile = new wxExMenu;
  menuFile->Append(wxID_OPEN);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(ID_LEXER_PROPERTIES, _("Lexer Properties"));
  menuFile->Append(ID_STATISTICS_SHOW, _("Show Statistics"));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxExMenu* menuConfig = new wxExMenu;
  menuConfig->Append(ID_CONFIG_DLG, wxExEllipsed(_("Config Dialog")));
  menuConfig->Append(
    ID_CONFIG_DLG_READONLY, 
    wxExEllipsed(_("Config Dialog Readonly")));

  wxExMenu* menuSTC = new wxExMenu;
  menuSTC->Append(ID_STC_FLAGS, wxExEllipsed(_("Open Flag")));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_CONFIG_DLG, wxExEllipsed(_("Config Dialog")));
  menuSTC->Append(ID_STC_ENTRY_DLG, wxExEllipsed(_("Entry Dialog")));
  menuSTC->Append(ID_STC_GOTO, wxExEllipsed(_("Goto Dialog")));
  menuSTC->Append(ID_STC_LEXER, wxExEllipsed(_("Lexer Dialog")));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_SPLIT, _("Split"));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_EDIT_MACRO_START_RECORD, _("Start Record"));
  menuSTC->Append(ID_EDIT_MACRO_STOP_RECORD, _("Stop Record"));
  menuSTC->Append(ID_EDIT_MACRO_PLAYBACK, _("Playback"));

  wxExMenu* menuHelp = new wxExMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, _("&File"));
  menubar->Append(menuSTC, _("&STC"));
  menubar->Append(menuConfig, _("&Config"));
  menubar->Append(menuHelp, _("&Help"));
  SetMenuBar(menubar);

  m_Notebook = new wxExNotebook(this, NULL);
#if wxUSE_GRID
  m_Grid = new wxExGrid(m_Notebook);
#endif
  m_ListView = new wxExListView(m_Notebook);
  m_STC = new wxExSTC(this);
  m_STCShell = new wxExSTCShell(this, ">", wxTextFile::GetEOL(), true, 10);

  GetManager().AddPane(m_STC, 
    wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true).Name("wxExSTC"));
  GetManager().AddPane(m_STCShell, wxAuiPaneInfo().Bottom().MinSize(wxSize(250, 250)));
  GetManager().AddPane(m_Notebook, wxAuiPaneInfo().Left().MinSize(wxSize(250, 250)));
  GetManager().AddPane(new wxExFindToolBar(this, this),
    wxAuiPaneInfo().ToolbarPane().Bottom().Name("FINDBAR").Caption(_("Findbar")));
  GetManager().Update();

  m_STCLexers = new wxExSTC(this, wxExLexers::Get()->GetFileName());
  m_Notebook->AddPage(m_STCLexers, wxExLexers::Get()->GetFileName().GetFullName());
  m_Notebook->AddPage(m_ListView, "wxExListView");

#if wxUSE_GRID
  m_Notebook->AddPage(m_Grid, "wxExGrid");
  m_Grid->CreateGrid(0, 0);
  m_Grid->AppendCols(2);
  wxExSampleDir dir(wxGetCwd(), "app.*", m_Grid);
  dir.FindFiles();
  m_Grid->AutoSizeColumns();
#endif

  m_ListView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  m_ListView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));
  m_ListView->InsertColumn(wxExColumn("Float", wxExColumn::COL_FLOAT));
  m_ListView->InsertColumn(wxExColumn("Date", wxExColumn::COL_DATE));

  const int items = 50;

  for (int i = 0; i < items; i++)
  {
    m_ListView->InsertItem(i, wxString::Format("item%d", i));
    m_ListView->SetItem(i, 1, wxString::Format("%d", i));
    m_ListView->SetItem(i, 2, wxString::Format("%f", (float)i / 2.0));
    m_ListView->SetItem(i, 3, wxDateTime::Now().Format());

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
  std::vector<wxExPane> panes;
  panes.push_back(wxExPane("PaneText", -3));
  panes.push_back(wxExPane("PaneFileType", 50, _("File type")));
  panes.push_back(wxExPane("PaneCells", 60, _("Cells")));
  panes.push_back(wxExPane("PaneItems", 60, _("Items")));
  panes.push_back(wxExPane("PaneLines", 100, _("Lines")));
  panes.push_back(wxExPane("PaneLexer", 60, _("Lexer")));
  SetupStatusBar(panes);
#endif

  CreateToolBar();
  m_ToolBar->AddTool(wxID_OPEN);
  m_ToolBar->AddTool(wxID_SAVE);
  m_ToolBar->AddTool(wxID_PRINT);
  m_ToolBar->AddTool(wxID_EXIT);
  m_ToolBar->Realize();
}

void wxExSampleFrame::ConfigDialogApplied(wxWindowID /* id */)
{
  m_STC->ConfigGet();
  m_STCLexers->ConfigGet();
  m_STCShell->ConfigGet();
}

wxExGrid* wxExSampleFrame::GetGrid()
{
  return m_Grid;
}

wxExListView* wxExSampleFrame::GetListView()
{
  return m_ListView;
}

wxExSTC* wxExSampleFrame::GetSTC()
{
  // First if we have a focus somewhere.
  if (m_STC->HasFocus())
  {
    return m_STC;
  }
  else if (m_STCShell->HasFocus())
  {
    return m_STCShell;
  }
  else if (m_STCLexers->HasFocus())
  {
    return m_STCLexers;
  }
  // Then if shown.
  else if (m_STC->IsShown())
  {
    return m_STC;
  }
  else if (m_STCShell->IsShown())
  {
    return m_STCShell;
  }
  else if (m_STCLexers->IsShown())
  {
    return m_STCLexers;
  }
  
  return NULL;
}
  
void wxExSampleFrame::OnCommand(wxCommandEvent& event)
{
  m_Statistics.Inc(wxString::Format("%d", event.GetId()));

  switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wxEX_VERSION_STRING);
    info.AddDeveloper(wxVERSION_STRING);
    info.SetCopyright("(c) 1998-2010 Anton van Wezenbeek");
    wxAboutBox(info);
    }
    break;
  case wxID_EXIT: Close(true); break;
  case wxID_OPEN:
    {
    wxExFileDialog dlg(this, m_STC);
    if (dlg.ShowModalIfChanged(true) == wxID_CANCEL) return;

    wxStopWatch sw;
    m_STC->Open(dlg.GetPath(), 0, wxEmptyString, m_FlagsSTC);
    const long stop = sw.Time();

#if wxUSE_STATUSBAR
    StatusText(wxString::Format(
      "wxExSTC::Open:%ld milliseconds, %d bytes", stop, m_STC->GetTextLength()));
#endif
    }
    break;

  case wxID_PREVIEW: m_ListView->PrintPreview(); break;
  case wxID_PRINT: m_ListView->Print(); break;
  case wxID_PRINT_SETUP: wxExPrinting::Get()->GetHtmlPrinter()->PageSetup(); break;

  case wxID_SAVE:
    m_STC->FileSave();

    if (m_STC->GetFileName().GetFullPath() == 
        wxExLexers::Get()->GetFileName().GetFullPath())
    {
      wxExLexers::Get()->Read();
      wxLogMessage("File contains: %d lexers", wxExLexers::Get()->Count());
        // As the lexer might have changed, update status bar field as well.
#if wxUSE_STATUSBAR
      m_STC->UpdateStatusBar("PaneLexer");
#endif
    }
    break;

  case ID_CONFIG_DLG: ShowConfigItems(); break;
  case ID_CONFIG_DLG_READONLY:
    {
    std::vector<wxExConfigItem> v;

    v.push_back(wxExConfigItem(_("File Picker"), CONFIG_FILEPICKERCTRL));

    for (size_t j = 1; j <= 10; j++)
    {
      v.push_back(wxExConfigItem(wxString::Format(_("Integer%d"), j), CONFIG_INT));
    }

    wxExConfigDialog* dlg = new wxExConfigDialog(
      this,
      v,
      _("Config Dialog Readonly"),
      0,
      2,
      wxCANCEL);

      dlg->Show();
    }
    break;

  case ID_LEXER_PROPERTIES:
    {
      wxString lexer;

      if (wxExLexers::Get()->ShowDialog(this, lexer))
      {
        wxString text;
        const wxExLexer l = wxExLexers::Get()->FindByName(lexer);
        
        for (
          std::vector<wxString>::const_iterator it = l.GetColourings().begin();
          it != l.GetColourings().end();
          ++it)
        {
          text += *it;
        }

        wxLogMessage(text);
      }
    }
    break;

  case ID_SHELL_COMMAND:
      m_STCShell->Prompt("Hello '" + event.GetString() + "' from the shell");
    break;

  case ID_STATISTICS_SHOW:
    m_Notebook->AddPage(m_Statistics.Show(m_Notebook), "Statistics");
    break;

  case ID_STC_CONFIG_DLG:
    wxExSTC::ConfigDialog(
      this,
      _("Editor Options"),
      wxExSTC::STC_CONFIG_MODELESS | wxExSTC::STC_CONFIG_WITH_APPLY);
    break;
    
  case ID_STC_ENTRY_DLG:
    {
    wxString text;
    
    for (int i = 0; i < 100; i++)
    {
      text += wxString::Format("Hello from line: %d\n", i);
    }
    
    wxExSTCEntryDialog dlg(
      this,
      "Hello world",
      text,      
      "Greetings from\nwxextension");
      
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
    
  case ID_STC_GOTO: m_STC->GotoDialog(); break;
  case ID_STC_LEXER: m_STC->LexerDialog(); break;
  
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

void wxExSampleFrame::ShowConfigItems()
{
  std::vector<wxExConfigItem> v;

  // CONFIG_CHECKLISTBOX
  std::map<long, const wxString> clb;
  clb.insert(std::make_pair(0, _("Bit One")));
  clb.insert(std::make_pair(1, _("Bit Two")));
  clb.insert(std::make_pair(2, _("Bit Three")));
  clb.insert(std::make_pair(4, _("Bit Four")));
  v.push_back(wxExConfigItem(
    _("Bin Choices"), 
    clb, 
    false, 
    "Lists"));

  // CONFIG_CHECKLISTBOX_NONAME
  std::set<wxString> bchoices;
  bchoices.insert(_("This"));
  bchoices.insert(_("Or"));
  bchoices.insert(_("Other"));
  v.push_back(wxExConfigItem(
    bchoices, 
    "Lists"));

  // CONFIG_RADIOBOX
  std::map<long, const wxString> echoices;
  echoices.insert(std::make_pair(0, _("Zero")));
  echoices.insert(std::make_pair(1, _("One")));
  echoices.insert(std::make_pair(2, _("Two")));
  v.push_back(wxExConfigItem(
    _("Radio Box"), 
    echoices, 
    true, 
    "Lists"));

  // CONFIG_CHECKBOX
  for (size_t h = 1; h <= 4; h++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format(_("Checkbox%d"), h), 
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

  // CONFIG_COLOUR
  for (size_t i = 1; i <= 5; i++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format(_("Colour%d"), i), 
      CONFIG_COLOUR, 
      "Colours"));
  }

  // CONFIG_COMBOBOX
  for (size_t m = 1; m <= 5; m++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format(_("Combobox%d"), m), 
      CONFIG_COMBOBOX, 
      "Comboboxes"));
  }

  // CONFIG_COMBOBOX_NONAME
  v.push_back(wxExConfigItem());
  v.push_back(wxExConfigItem(
    _("Combobox No Name"),
    CONFIG_COMBOBOX_NONAME, 
    "Comboboxes"));

  // CONFIG_COMBOBOXDIR
  v.push_back(wxExConfigItem(
    _("Combobox Dir"), 
    CONFIG_COMBOBOXDIR, 
    "Comboboxes"));

  // CONFIG_DIRPICKERCTRL
  v.push_back(wxExConfigItem(
    _("Dir Picker"), 
    CONFIG_DIRPICKERCTRL, 
    "Pickers"));

  // CONFIG_FILEPICKERCTRL
  v.push_back(wxExConfigItem(
    _("File Picker"), 
    CONFIG_FILEPICKERCTRL, 
    "Pickers"));

  // CONFIG_FONTPICKERCTRL
  v.push_back(wxExConfigItem(
    _("Font Picker"), 
    CONFIG_FONTPICKERCTRL, 
    "Pickers"));

  // CONFIG_INT
  for (size_t j = 1; j <= 5; j++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format(_("Integer%d"), j), 
      CONFIG_INT, 
      "Integers", 
      true));
  }

  // CONFIG_SPINCTRL
  for (size_t s = 1; s <= 3; s++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format(_("Spin Control%d"), s), 
      1, 
      s, 
      wxString("Spin controls")));
  }

  // CONFIG_SPINCTRL_DOUBLE
  for (size_t sd = 1; sd <= 3; sd++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format(_("Spin Control Double%d"), sd), 
      1, 
      sd, 
      1, 
      "Spin controls"));
  }

  // CONFIG_STRING
  for (size_t l = 1; l <= 5; l++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format(_("String%d"), l), 
      CONFIG_STRING, 
      "Strings"));

    // CONFIG_SPACER
    v.push_back(wxExConfigItem());
    v.push_back(wxExConfigItem());
  }

  wxExConfigDialog* dlg = new wxExConfigDialog(
    this,
    v,
    _("Config Dialog"),
    10,
    2,
    wxAPPLY | wxCANCEL,
    wxID_ANY,
    wxDefaultPosition,
    wxSize(400,300));

  dlg->ForceCheckBoxChecked("Group", "Checkboxes");
  dlg->Show();
}
