/******************************************************************************\
* File:          appl.cpp
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
#include <wx/extension/renderer.h>
#include "appl.h"
#ifndef __WXMSW__
#include "appl.xpm"
#endif

enum
{
  ID_FIRST = 15000,
  ID_CONFIG_DLG,
  ID_CONFIG_DLG_READONLY,
  ID_LOCALE_SHOW_DIR,
  ID_STATISTICS_SHOW,
  ID_STC_CONFIG_DLG,
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

  SetLogging();

  wxExSampleFrame *frame = new wxExSampleFrame();
  frame->Show(true);

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
  SetIcon(wxICON(appl));

  wxExMenu* menuFile = new wxExMenu;
  menuFile->Append(wxID_OPEN);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(ID_LOCALE_SHOW_DIR, _("Show Locale Dir"));
  menuFile->AppendSeparator();
  menuFile->Append(ID_STATISTICS_SHOW, _("Show Statistics"));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxExMenu* menuConfig = new wxExMenu;
  menuConfig->Append(ID_CONFIG_DLG, wxExEllipsed(_("Config Dialog")));
  menuConfig->Append(ID_CONFIG_DLG_READONLY, wxExEllipsed(_("Config Dialog Readonly")));

  wxExMenu* menuSTC = new wxExMenu;
  menuSTC->Append(ID_STC_FLAGS, wxExEllipsed(_("Open Flag")));
  menuSTC->AppendSeparator();
  menuSTC->Append(ID_STC_CONFIG_DLG, wxExEllipsed(_("Config Dialog")));
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

  GetManager().AddPane(m_STC, wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true).Name("wxExSTC"));
  GetManager().AddPane(m_STCShell, wxAuiPaneInfo().Bottom().MinSize(wxSize(250, 250)));
  GetManager().AddPane(m_Notebook, wxAuiPaneInfo().Left().MinSize(wxSize(250, 250)));
  GetManager().Update();

  assert(wxExApp::GetLexers());

  wxExSTC* st = new wxExSTC(this, wxExApp::GetLexers()->GetFileName());
  m_Notebook->AddPage(st, wxExApp::GetLexers()->GetFileName().GetFullName());
  m_Notebook->AddPage(m_ListView, "wxExListView");

#if wxUSE_GRID
  m_Notebook->AddPage(m_Grid, "wxExGrid");
  m_Grid->CreateGrid(0, 0);
  m_Grid->AppendCols(2);
  wxExSampleDir dir(wxGetCwd(), "appl.*", m_Grid);
  dir.FindFiles();
  m_Grid->AutoSizeColumns();
#endif

  m_ListView->SetSingleStyle(wxLC_REPORT); // wxLC_ICON);
  m_ListView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  m_ListView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));
  m_ListView->InsertColumn(wxExColumn("Float", wxExColumn::COL_FLOAT));
  m_ListView->InsertColumn(wxExColumn("Date", wxExColumn::COL_DATE));

  const int items = 50;

  for (int i = 0; i < items; i++)
  {
    wxExListItem item(m_ListView, wxString::Format("item%d", i));
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
  if (m_STC->IsShown())
  {
    return m_STC;
  }
  else if (m_STCShell->IsShown())
  {
    return m_STCShell;
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
    info.SetCopyright(_("(c) 1998-2009 Anton van Wezenbeek."));
    wxAboutBox(info);
    }
    break;
  case wxID_EXIT: Close(true); break;
  case wxID_OPEN:
    {
    wxExFileDialog dlg(this, m_STC);
    if (dlg.ShowModal() == wxID_CANCEL) return;

    wxStopWatch sw;
    m_STC->Open(dlg.GetPath(), 0, wxEmptyString, m_FlagsSTC);
    const long stop = sw.Time();

#if wxUSE_STATUSBAR
    StatusText(wxString::Format("wxExSTC::Open:%ld milliseconds, %d bytes", stop, m_STC->GetTextLength()));
#endif
    }
    break;

  case wxID_PREVIEW: m_ListView->PrintPreview(); break;
  case wxID_PRINT: m_ListView->Print(); break;
  case wxID_PRINT_SETUP: wxExApp::GetPrinter()->PageSetup(); break;

  case wxID_SAVE:
    m_STC->FileSave();

    if (m_STC->GetFileName().GetFullPath() == wxExApp::GetLexers()->GetFileName().GetFullPath())
    {
      if (wxExApp::GetLexers()->Read())
      {
        wxLogMessage("File contains: %d lexers", wxExApp::GetLexers()->Count());
        // As the lexer might have changed, update status bar field as well.
#if wxUSE_STATUSBAR
        m_STC->UpdateStatusBar("PaneLexer");
#endif
      }
    }
    break;

  case ID_CONFIG_DLG:
    {
    std::vector<wxExConfigItem> v;

    for (size_t h = 1; h <= 25; h++)
    {
      v.push_back(wxExConfigItem(wxString::Format(_("check%d"), h), CONFIG_CHECKBOX, "Checkboxes"));
    }

    for (size_t i = 1; i <= 25; i++)
    {
      v.push_back(wxExConfigItem(wxString::Format(_("colour%d"), i), CONFIG_COLOUR, "Colours"));
    }

    for (size_t j = 1; j <= 10; j++)
    {
      v.push_back(wxExConfigItem(wxString::Format(_("integer%d"), j), CONFIG_INT, "Integers", true));
    }

    for (size_t k = 1; k <= 10; k++)
    {
      v.push_back(wxExConfigItem(wxString::Format(_("spin%d"), k), 1, k, wxString("Spin controls")));
    }

    for (size_t l = 1; l <= 10; l++)
    {
      v.push_back(wxExConfigItem(wxString::Format(_("string%d"), l), CONFIG_STRING, "Strings"));
    }

    for (size_t m = 1; m <= 10; m++)
    {
      v.push_back(wxExConfigItem(wxString::Format(_("combobox%d"), m), CONFIG_COMBOBOX, "Comboboxes"));
    }

    v.push_back(wxExConfigItem(_("dirpicker"), CONFIG_DIRPICKERCTRL, "Pickers"));
    v.push_back(wxExConfigItem(_("filepicker"), CONFIG_FILEPICKERCTRL, "Pickers"));

    wxExConfigDialog* dlg = new wxExConfigDialog(
      this,
      wxExApp::GetConfig(),
      v,
      _("Config Dialog"),
      wxEmptyString,
      10,
      6,
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
    std::vector<wxExConfigItem> v;

    v.push_back(wxExConfigItem(_("filepicker"), CONFIG_FILEPICKERCTRL));

    for (size_t j = 1; j <= 10; j++)
    {
      v.push_back(wxExConfigItem(wxString::Format(_("integer%d"), j), CONFIG_INT));
    }

    wxExConfigDialog* dlg = new wxExConfigDialog(
      this,
      wxExApp::GetConfig(),
      v,
      _("Config Dialog Readonly"),
      wxEmptyString,
      0,
      2,
      wxCANCEL);

      dlg->Show();
    }
    break;

  case ID_LOCALE_SHOW_DIR:
    wxLogMessage(wxExApp::GetCatalogDir());
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
