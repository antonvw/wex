/******************************************************************************\
* File:          appl.cpp
* Purpose:       Implementation of sample classes for wxextension
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/aboutdlg.h>
#include <wx/busyinfo.h>
#include <wx/numdlg.h>
#include <wx/stdpaths.h>
#include <wx/extension/renderer.h>
#include "appl.h"
#include "appl.xpm"

enum
{
  ID_FIRST = 15000,
  ID_CONFIG_DLG,
  ID_CONFIG_DLG_READONLY,
  ID_CONFIG_TIMING,
  ID_FILE_TIMING,
  ID_FILE_ATTRIB_TIMING,
  ID_PRINT_SPECIAL,
  ID_LOCALE_SHOW_DIR,
  ID_STATISTICS_CLEAR,
  ID_STATISTICS_SHOW,
  ID_STC_CONFIG_DLG,
  ID_STC_FLAGS,
  ID_STC_GOTO,
  ID_STC_SPLIT,
  ID_STC_LEXER,
  ID_LAST,
};

BEGIN_EVENT_TABLE(exSampleFrame, exFrame)
  EVT_MENU_RANGE(wxID_LOWEST, wxID_HIGHEST, exSampleFrame::OnCommand)
  EVT_MENU_RANGE(ID_FIRST, ID_LAST, exSampleFrame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND, exSampleFrame::OnCommand)
END_EVENT_TABLE()

IMPLEMENT_APP(exSampleApp)

bool exSampleApp::OnInit()
{
  SetAppName("exSample");
  SetLogging();

  exApp::OnInit();

  exSampleFrame *frame = new exSampleFrame("exSample");
  frame->Show(true);

  SetTopWindow(frame);

  return true;
}

#if wxUSE_GRID
exSampleDir::exSampleDir(const wxString& fullpath, const wxString& findfiles, exGrid* grid)
  : exDir(fullpath, findfiles)
  , m_Grid(grid)
{
}

void exSampleDir::OnFile(const wxString& file)
{
  m_Grid->AppendRows(1);
  const int no = m_Grid->GetNumberRows() - 1;
  m_Grid->SetCellValue(no, 0, wxString::Format("cell%d", no));
  m_Grid->SetCellValue(no, 1, file);

  exRenderer* renderer = new exRenderer(exRenderer::CELL_CROSS, *wxGREEN_PEN, *wxRED_PEN);
  m_Grid->SetCellRenderer(no, 0, renderer);

  // Let's make these cells readonly and colour them, so we can test
  // things like cutting and dropping is forbidden.
  m_Grid->SetReadOnly(no, 1);
  m_Grid->SetCellBackgroundColour(no, 1, *wxLIGHT_GREY);
}
#endif

exSampleFrame::exSampleFrame(const wxString& title)
  : exManagedFrame(NULL, wxID_ANY, title)
  , m_FlagsSTC(0)
{
  SetIcon(appl_xpm);

  exMenu* menuFile = new exMenu;
  menuFile->Append(wxID_OPEN);
  menuFile->AppendSeparator();
  menuFile->Append(ID_FILE_TIMING, _("Timing"));
  menuFile->Append(ID_FILE_ATTRIB_TIMING, _("Attrib Timing"));
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(ID_PRINT_SPECIAL, _("Print Test")); // test to print without a window
  menuFile->AppendSeparator();
  menuFile->Append(ID_LOCALE_SHOW_DIR, _("Show Locale Dir"));
  menuFile->AppendSeparator();
  menuFile->Append(ID_STATISTICS_SHOW, _("Show Statistics"));
  menuFile->Append(ID_STATISTICS_CLEAR, _("Clear Statistics"));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  exMenu* menuConfig = new exMenu;
  menuConfig->Append(ID_CONFIG_DLG, exEllipsed(_("Config Dialog")));
  menuConfig->Append(ID_CONFIG_DLG_READONLY, exEllipsed(_("Config Dialog Readonly")));
  menuConfig->AppendSeparator();
  menuConfig->Append(ID_CONFIG_TIMING, _("Timing"));

  exMenu* menuSTC = new exMenu;
  menuSTC->Append(ID_STC_FLAGS, exEllipsed(_("Open Flag")));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_CONFIG_DLG, exEllipsed(_("Config Dialog")));
  menuSTC->Append(ID_STC_GOTO, exEllipsed(_("Goto Dialog")));
  menuSTC->Append(ID_STC_LEXER, exEllipsed(_("Lexer Dialog")));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_SPLIT, _("Split"));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_EDIT_MACRO_START_RECORD, _("Start Record"));
  menuSTC->Append(ID_EDIT_MACRO_STOP_RECORD, _("Stop Record"));
  menuSTC->Append(ID_EDIT_MACRO_PLAYBACK, _("Playback"));

  exMenu* menuHelp = new exMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, _("&File"));
  menubar->Append(menuSTC, _("&STC"));
  menubar->Append(menuConfig, _("&Config"));
  menubar->Append(menuHelp, _("&Help"));
  SetMenuBar(menubar);

  m_Notebook = new exNotebook(this, NULL);
#if wxUSE_GRID
  m_Grid = new exGrid(m_Notebook);
#endif
  m_ListView = new exListView(m_Notebook);
  exSTC* st = new exSTC(this);
  m_STCShell = new exSTCShell(this, ">", wxTextFile::GetEOL(), true, 10);

  GetManager().AddPane(st, wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true).Name("exSTC"));
  GetManager().AddPane(m_STCShell, wxAuiPaneInfo().Bottom().MinSize(wxSize(250, 250)));
  GetManager().AddPane(m_Notebook, wxAuiPaneInfo().Left().MinSize(wxSize(250, 250)));
  GetManager().Update();

  assert(exApp::GetLexers());
  
  m_STC = new exSTC(this, exApp::GetLexers()->GetFileName().GetFullPath());
  m_Notebook->AddPage(m_STC, exApp::GetLexers()->GetFileName().GetFullName());
  m_Notebook->AddPage(m_ListView, "exListView");
  
#if wxUSE_GRID
  m_Notebook->AddPage(m_Grid, "exGrid");
  m_Grid->CreateGrid(0, 0);
  m_Grid->AppendCols(2);
  exSampleDir dir(wxGetCwd(), "appl.*", m_Grid);
  dir.FindFiles();
  m_Grid->AutoSizeColumns();
#endif

  m_ListView->SetSingleStyle(wxLC_REPORT); // wxLC_ICON);
  m_ListView->InsertColumn("String", exColumn::COL_STRING);
  m_ListView->InsertColumn("Number", exColumn::COL_INT);
  m_ListView->InsertColumn("Float", exColumn::COL_FLOAT);
  m_ListView->InsertColumn("Date", exColumn::COL_DATE);

  const int items = 50;

  for (int i = 0; i < items; i++)
  {
    exListItem item(m_ListView, wxString::Format("item%d", i));
    item.Insert();
    item.SetColumnText(1, wxString::Format("%d", i));
    item.SetColumnText(2, wxString::Format("%f", (float)i / 2.0));
    item.SetColumnText(3, wxDateTime::Now().Format());

    // Set some images.
    if      (i == 0) item.SetImage(wxART_CDROM);
    else if (i == 1) item.SetImage(wxART_REMOVABLE);
    else if (i == 2) item.SetImage(wxART_FOLDER);
    else if (i == 3) item.SetImage(wxART_FOLDER_OPEN);
    else if (i == 4) item.SetImage(wxART_GO_DIR_UP);
    else if (i == 5) item.SetImage(wxART_EXECUTABLE_FILE);
    else if (i == 6) item.SetImage(wxART_NORMAL_FILE);
    else             item.SetImage(wxART_TICK_MARK);
  }

  std::vector<exPane> panes;
  panes.push_back(exPane("PaneText", -3));
  panes.push_back(exPane("PaneFileType", 50, _("File type")));
  panes.push_back(exPane("PaneCells", 60, _("Cells")));
  panes.push_back(exPane("PaneItems", 60, _("Items")));
  panes.push_back(exPane("PaneLines", 100, _("Lines")));
  panes.push_back(exPane("PaneLexer", 60, _("Lexer")));
  SetupStatusBar(panes);

  m_ToolBar = new exToolBar(this);
  m_ToolBar->AddTool(wxID_OPEN);
  m_ToolBar->AddTool(wxID_SAVE);
  m_ToolBar->AddTool(wxID_PRINT);
  m_ToolBar->AddTool(wxID_EXIT);
  m_ToolBar->Realize();
  SetToolBar(m_ToolBar);
}

void exSampleFrame::ConfigDialogApplied(wxWindowID id)
{
  m_STC->ConfigGet();
  m_STCShell->ConfigGet();
}

void exSampleFrame::OnCommand(wxCommandEvent& event)
{
  m_Statistics.Inc(wxString::Format("%d", event.GetId()));

  switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(EX_LIB_VERSION);
    info.AddDeveloper(wxVERSION_STRING);
    info.SetCopyright(_("Copyright (c) Anton van Wezenbeek."));
    wxAboutBox(info);
    }
    break;
  case wxID_EXIT: Close(true); break;
  case wxID_OPEN:
    {
    wxFileDialog dlg(this,
      _("Select File"),
      wxEmptyString,
      wxEmptyString,
      wxFileSelectorDefaultWildcardStr,
      wxFD_OPEN | wxFD_CHANGE_DIR);

    if (m_STC->AskFileOpen(dlg) == wxID_CANCEL) return;

    wxStopWatch sw;
    m_STC->Open(dlg.GetPath(), 0, wxEmptyString, m_FlagsSTC);
    const long stop = sw.Time();

    StatusText(wxString::Format("exSTC::Open:%ld milliseconds, %d bytes", stop, m_STC->GetTextLength()));
    }
    break;

  case wxID_PREVIEW: m_ListView->PrintPreview(); break;
  case wxID_PRINT: m_ListView->Print(); break;
  case wxID_PRINT_SETUP: exApp::GetPrinter()->PageSetup(); break;

  case wxID_SAVE: 
    m_STC->FileSave();

    if (m_STC->GetFileName().GetFullPath() == exApp::GetLexers()->GetFileName().GetFullPath())
    {
      exApp::GetLexers()->Read();

      m_STC->SetLexer();
      // As the lexer might have changed, update status bar field as well.
      m_STC->UpdateStatusBar("PaneLexer");
    }
    break;

  case ID_CONFIG_DLG:
    {
    std::vector<exConfigItem> v;

    for (size_t h = 1; h <= 25; h++)
    {
      v.push_back(exConfigItem(wxString::Format(_("check%d"), h), CONFIG_CHECKBOX, "Checkboxes"));
    }

    for (size_t i = 1; i <= 25; i++)
    {
      v.push_back(exConfigItem(wxString::Format(_("colour%d"), i), CONFIG_COLOUR, "Colours"));
    }

    for (size_t j = 1; j <= 10; j++)
    {
      v.push_back(exConfigItem(wxString::Format(_("integer%d"), j), CONFIG_INT, "Integers", true));
    }

    for (size_t k = 1; k <= 10; k++)
    {
      v.push_back(exConfigItem(wxString::Format(_("spin%d"), k), 1, k, wxString("Spin controls")));
    }

    for (size_t l = 1; l <= 10; l++)
    {
      v.push_back(exConfigItem(wxString::Format(_("string%d"), l), CONFIG_STRING, "Strings"));
    }

    for (size_t m = 1; m <= 10; m++)
    {
      v.push_back(exConfigItem(wxString::Format(_("combobox%d"), m), CONFIG_COMBOBOX, "Comboboxes"));
    }

    v.push_back(exConfigItem(_("dirpicker"), CONFIG_DIRPICKERCTRL, "Pickers"));
    v.push_back(exConfigItem(_("filepicker"), CONFIG_FILEPICKERCTRL, "Pickers"));

    exConfigDialog* dlg = new exConfigDialog(
      this,
      v,
      _("Config Dialog"),
      wxEmptyString,
      5,
      5,
      wxAPPLY | wxCANCEL,
      wxID_ANY,
      wxDefaultPosition,
      wxSize(400,300));

      dlg->Show();

      // Dialog is not placed nicely.
      //GetManager().GetPane("NOTEBOOK"));
      //GetManager().Update();
    }
    break;

  case ID_CONFIG_DLG_READONLY:
    {
    std::vector<exConfigItem> v;

    v.push_back(exConfigItem(_("filepicker"), CONFIG_FILEPICKERCTRL));

    for (size_t j = 1; j <= 10; j++)
    {
      v.push_back(exConfigItem(wxString::Format(_("integer%d"), j), CONFIG_INT));
    }

    exConfigDialog* dlg = new exConfigDialog(
      this,
      v,
      _("Config Dialog Readonly"),
      wxEmptyString,
      1,
      1,
      wxCANCEL);

      dlg->Show();
    }
    break;

  case ID_CONFIG_TIMING:
    {
    wxBusyInfo wait(_("Please wait, working..."));
    wxTheApp->Yield();

    const int max = 100000;

    wxStopWatch sw;

    for (int i = 0; i < max; i++)
    {
      exApp::GetConfig("test", 0);
    }

    const long exconfig = sw.Time();

    sw.Start();

    for (int j = 0; j < max; j++)
    {
      exApp::GetConfig()->Read("test", 0l);
    }

    const long config = sw.Time();

    StatusText(wxString::Format(
      "exConfig::Get:%ld wxConfig::Read:%ld",
      exconfig,
      config));
    }
    break;

  case ID_FILE_TIMING:
    {
    wxBusyInfo wait(_("Please wait, working..."));

    exFile file(m_STC->GetFileName().GetFullPath());

    if (!file.IsOpened())
    {
      wxLogError("File could not be opened");
      return;
    }

    const int max = 10000;

    wxStopWatch sw;

    for (int i = 0; i < max; i++)
    {
      wxString* buffer = file.Read();
      delete buffer;
    }

    const long exfile_read = sw.Time();

    sw.Start();

    wxFile wxfile(m_STC->GetFileName().GetFullPath());

    for (int j = 0; j < max; j++)
    {
      char* charbuffer= new char[wxfile.Length()];
      wxfile.Read(charbuffer, wxfile.Length());
      wxString* buffer = new wxString(charbuffer, wxfile.Length());
      delete charbuffer;
      delete buffer;
    }

    const long file_read = sw.Time();

    StatusText(wxString::Format(
      "exFile::Read:%ld wxFile::Read:%ld",
      exfile_read,
      file_read));
    }
    break;

  case ID_FILE_ATTRIB_TIMING:
    {
    wxBusyInfo wait(_("Please wait, working..."));

    const int max = 1000;

    wxStopWatch sw;

    const exFileName exfile(m_STC->GetFileName().GetFullPath());

    int checked = 0;

    for (int i = 0; i < max; i++)
    {
      checked += exfile.GetStat().IsReadOnly();
    }

    const long exfile_time = sw.Time();

    sw.Start();

    const wxFileName file(m_STC->GetFileName().GetFullPath());

    for (int j = 0; j < max; j++)
    {
      checked += file.IsFileWritable();
    }

    const long file_time = sw.Time();

    StatusText(wxString::Format(
      "exFileName::IsReadOnly:%ld wxFileName::IsFileWritable:%ld",
      exfile_time,
      file_time));
    }
    break;

  case ID_PRINT_SPECIAL:
    {
    wxHtmlEasyPrinting* print = exApp::GetPrinter();

    wxPrintDialogData printDialogData(*print->GetPrintData());
    wxPrinter printer(&printDialogData);

    wxHtmlPrintout printout;
    printout.SetHtmlFile("appl.xpm");
    printer.Print(this, &printout, false);

    /*
    // This is the simplest, but
    // asks for printing always (in lib source).
    print->PrintFile("mondrian.xpm");
    */

    /*
    // This could be made better, immediately hide etc.
    exSTC* stc = new exSTC(this, "mondrian.xpm");
    stc->Hide();
    stc->Print(false);// however stc->Print(false) does not print
    delete stc;
    */
    }
    break;

  case ID_LOCALE_SHOW_DIR:
    wxLogMessage(exApp::GetCatalogDir());
    break;

  case ID_SHELL_COMMAND:
      m_STCShell->AppendText("\nHello '" + event.GetString() + "' from the shell"),
      m_STCShell->Prompt();
    break;

  case ID_STATISTICS_SHOW:
    m_Notebook->AddPage(m_Statistics.Show(m_Notebook), "Statistics");
    break;
  case ID_STATISTICS_CLEAR:
    m_Statistics.Clear();
    break;

  case ID_STC_CONFIG_DLG:
    exSTC::ConfigDialog(_("Editor Options"));
    break;
  case ID_STC_FLAGS:
    {
    long value = wxGetNumberFromUser(
      "Input:",
      wxEmptyString,
      "STC Open Flag",
      m_FlagsSTC,
      0,
      2 * exSTC::STC_OPEN_FROM_URL);

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
    exSTC* stc = new exSTC(*m_STC);
    m_Notebook->AddPage(
      stc, 
      wxString::Format("stc%d", stc->GetId()),
      m_STC->GetFileName().GetFullName());
    stc->SetDocPointer(m_STC->GetDocPointer());
    }
    break;
  }
}
