/******************************************************************************\
* File:          appl.cpp
* Purpose:       Implementation of classes for syncodbcquery
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/aboutdlg.h>
#include <wx/tokenzr.h>
#include <wx/extension/grid.h>
#include <wx/extension/shell.h>
#include "appl.h"
#include "appl.xpm"

const wxString Quoted(const wxString& text)
{
  return "'" + text + "'";
}

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
  SetAppName("syncodbcquery");

  exApp::OnInit();

  MyFrame *frame = new MyFrame("syncodbcquery");
  frame->Show(true);

  SetTopWindow(frame);

  return true;
}

BEGIN_EVENT_TABLE(MyFrame, ftFrame)
  EVT_CLOSE(MyFrame::OnClose)
  EVT_MENU(ID_SHELL_COMMAND, MyFrame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND_STOP, MyFrame::OnCommand)
  EVT_MENU_RANGE(wxID_LOWEST, wxID_HIGHEST, MyFrame::OnCommand)
  EVT_MENU_RANGE(ID_FIRST, ID_LAST, MyFrame::OnCommand)
  EVT_UPDATE_UI(wxID_SAVE, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_SAVEAS, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_DATABASE_CLOSE, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_DATABASE_OPEN, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_QUERY_RUN, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENTFILE_MENU, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_QUERY, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_RESULTS, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_STATISTICS, MyFrame::OnUpdateUI)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title)
  : ftFrame(NULL, wxID_ANY, title)
  , m_Stopped(false)
{
  SetIcon(appl_xpm);

  exMenu* menuFile = new exMenu;
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  exMenu* menuDatabase = new exMenu;
  menuDatabase->Append(ID_DATABASE_OPEN, exEllipsed(_("&Open")));
  menuDatabase->Append(ID_DATABASE_CLOSE, _("&Close"));

  exMenu* menuQuery = new exMenu;
  menuQuery->Append(ID_QUERY_RUN, _("&Run"), wxEmptyString, wxITEM_NORMAL, NULL, wxART_GO_FORWARD);
  menuQuery->Append(wxID_STOP);

  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(ID_OPTIONS, exEllipsed(_("&Edit")));

  wxMenu* menuView = new wxMenu();
  menuView->Append(ID_VIEW_STATUSBAR, _("&Statusbar"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_TOOLBAR, _("&Toolbar"), wxEmptyString, wxITEM_CHECK);
  menuView->AppendSeparator();
  menuView->Append(ID_VIEW_QUERY, _("Query"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_RESULTS, _("Results"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_STATISTICS, _("Statistics"), wxEmptyString, wxITEM_CHECK);

  wxMenu* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, _("&File"));
  menubar->Append(menuView, _("&View"));
  menubar->Append(menuDatabase, _("&Database"));
  menubar->Append(menuQuery, _("&Query"));
  menubar->Append(menuOptions, _("&Options"));
  menubar->Append(menuHelp, _("&Help"));
  SetMenuBar(menubar);

  m_Query = new ftSTC(this,
    exSTC::STC_MENU_SIMPLE | exSTC::STC_MENU_FIND |
    exSTC::STC_MENU_REPLACE | exSTC::STC_MENU_INSERT);
  m_Query->SetLexer("sql");

  m_Results = new exGrid(this);
  m_Results->CreateGrid(0, 0);
  m_Results->EnableEditing(false); // this is a read-only grid

  m_Shell = new exSTCShell(this, ">", ";", true, 50);
  m_Shell->SetFocus();
  m_Shell->SetLexer();

  GetManager().AddPane(m_Shell,
    wxAuiPaneInfo().
      Name("CONSOLE").
      CenterPane());

  GetManager().AddPane(m_Results,
    wxAuiPaneInfo().
      Name("RESULTS").
      Caption(_("Results")).
      CloseButton(true).
      MaximizeButton(true));

  GetManager().AddPane(m_Query,
    wxAuiPaneInfo().
      Name("QUERY").
      Caption(_("Query")).
      CloseButton(true).
      MaximizeButton(true));

  GetManager().AddPane(m_Statistics.Show(this),
    wxAuiPaneInfo().Left().
      MaximizeButton(true).
      Caption(_("Statistics")).
      Name("STATISTICS"));

  GetManager().LoadPerspective(exApp::GetConfig("Perspective"));
  GetManager().GetPane("QUERY").Show(false);

  GetManager().Update();

  std::vector<exPane> panes;
  panes.push_back(exPane("PaneText", -3));
  panes.push_back(exPane("PaneLines", 100, _("Lines in window")));
  SetupStatusBar(panes);

  exToolBar* toolbar = new exToolBar(this,
    wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTB_FLAT | wxTB_NODIVIDER);
  toolbar->AddTool(wxID_NEW);
  toolbar->AddTool(wxID_OPEN);
  toolbar->AddTool(wxID_SAVE);
  toolbar->AddTool(ID_QUERY_RUN, wxEmptyString, wxArtProvider::GetBitmap(wxART_GO_FORWARD), _("Run query"));
  toolbar->Realize();
  SetToolBar(toolbar);

  otl_connect::otl_initialize();
}

void MyFrame::ConfigDialogApplied(wxWindowID dialogid)
{
  m_Query->ConfigGet();
  m_Shell->ConfigGet();
}

void MyFrame::OnClose(wxCloseEvent& event)
{
  if (!m_Query->Continue())
  {
    return;
  }

  exApp::SetConfig("Perspective", GetManager().SavePerspective());

  m_db.logoff();

  event.Skip();
}

void MyFrame::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetDescription(_("This program offers a general ODBC query."));
    info.SetVersion("v1.0");
    info.SetCopyright("(c) 2008, Anton van Wezenbeek");
    info.AddDeveloper(wxVERSION_STRING);
    info.AddDeveloper(EX_LIB_VERSION);
    info.AddDeveloper(FT_LIB_VERSION);
    info.AddDeveloper(exOTLVersion());
    wxAboutBox(info);
    }
    break;

  case wxID_EXIT:
    Close(true);
    break;

  case wxID_NEW:
    if (m_Query->FileNew())
    {
      m_Query->SetLexer("sql");
      m_Query->SetFocus();
      GetManager().GetPane("QUERY").Show();
      GetManager().Update();
    }
    break;

  case wxID_OPEN:
    DialogFileOpen(wxFD_OPEN | wxFD_CHANGE_DIR, true);
    break;

  case wxID_SAVE:
    m_Query->FileSave();
    break;

  case wxID_SAVEAS:
    m_Query->FileSaveAs();
    break;

  case wxID_STOP:
    m_Stopped = true;
    break;

  case ID_DATABASE_CLOSE:
    m_db.logoff();
    m_Shell->SetPrompt(">");
    break;

  case ID_DATABASE_OPEN:
    exOTLDialog(&m_db);
    m_Shell->SetPrompt((m_db.connected ? exApp::GetConfig(_("Datasource")): "") + ">");
    break;

  case ID_OPTIONS:
    exSTC::ConfigDialog(_("Editor Options"), 
      exSTC::STC_CONFIG_SIMPLE | exSTC::STC_CONFIG_MODELESS);
    break;

  case ID_QUERY_RUN:
    m_Stopped = false;
    RunQueries(m_Query->GetText());
    break;

  case ID_SHELL_COMMAND:
    if (m_db.connected)
    {
      try
      {
        const wxString query = event.GetString().substr(
          0,
          event.GetString().length() - 1);

        m_Stopped = false;
        RunQuery(query, true);
      }
      catch (otl_exception& p)
      {
        if (m_Results->IsShown())
        {
          m_Results->EndBatch();
        }

        m_Shell->AppendText(_("\nerror: ") + Quoted(p.msg));
      }
    }
    else
    {
      m_Shell->AppendText(_("\nnot connected"));
    }

    m_Shell->Prompt();
    break;

  case ID_SHELL_COMMAND_STOP:
    m_Stopped = true;
    m_Shell->Prompt("Cancelled");
    break;

  case ID_VIEW_QUERY: TogglePane("QUERY"); break;
  case ID_VIEW_RESULTS: TogglePane("RESULTS"); break;
  case ID_VIEW_STATISTICS: TogglePane("STATISTICS"); break;

  default: event.Skip();
  }
}

void MyFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
  case wxID_SAVE:
    event.Enable(m_Query->GetModify());
    break;

  case wxID_SAVEAS:
    event.Enable(m_Query->GetLength() > 0);
    break;

  case ID_DATABASE_CLOSE:
    event.Enable(m_db.connected);
    break;

  case ID_DATABASE_OPEN:
    event.Enable(!m_db.connected);
    break;

  case ID_QUERY_RUN:
    // If we have a query, you can hide it, but still run it.
    event.Enable(m_Query->GetLength() > 0 && m_db.connected);
    break;

  case ID_RECENTFILE_MENU: 
    event.Enable(!GetRecentFile().empty()); 
    break;

  case ID_VIEW_QUERY:
    event.Check(GetManager().GetPane("QUERY").IsShown());
    break;

  case ID_VIEW_RESULTS:
    event.Check(GetManager().GetPane("RESULTS").IsShown());
    break;

  case ID_VIEW_STATISTICS:
    event.Check(GetManager().GetPane("STATISTICS").IsShown());
    break;

  default:  
    wxLogError(wxString::Format("Unhandled event: %d"), event.GetId());
  }
}

bool MyFrame::OpenFile(const wxString& file,
  int line_number,
  const wxString& match,
  long flags)
{
  GetManager().GetPane("QUERY").Show(true);
  GetManager().Update();

  // Take care that DialogFileOpen always results in opening in the query.
  // Otherwise if results are focused, the file is opened in the results.
  return m_Query->Open(file, line_number, match, flags);
}

void MyFrame::RunQuery(const wxString& query, bool empty_results)
{
  wxStopWatch sw;

  const wxString query_lower = query.Lower();

  // Query functions supported by ODBC
  // $SQLTables, $SQLColumns, etc.
  // $SQLTables $1:'%'
  // allow you to get database schema.
  if (query_lower.StartsWith("select") ||
      query_lower.StartsWith("describe") ||
      query_lower.StartsWith("show") ||
      query_lower.StartsWith("explain") ||
      query_lower.StartsWith("$sql"))
  {
    long rpc;

    if (m_Results->IsShown())
    {
      rpc = exOTLQueryToGrid(&m_db, query, m_Results, m_Stopped, empty_results);
    }
    else
    {
      rpc = exOTLQueryToSTC(&m_db, query, m_Shell, m_Stopped);
    }

    sw.Pause();

    UpdateStatistics(sw, rpc);
  }
  else
  {
    const long rpc = otl_cursor::direct_exec(m_db, query.c_str());
    sw.Pause();

    UpdateStatistics(sw, rpc);
  }

  m_Shell->DocumentEnd();
}

void MyFrame::RunQueries(const wxString& text)
{
  if (m_Results->IsShown())
  {
    m_Results->ClearGrid();
  }

  // Skip sql comments.
  wxString output = text;
  wxRegEx("--.*$", wxRE_NEWLINE).ReplaceAll(&output, "");

  // Queries are seperated by ; character.
  wxStringTokenizer tkz(output, ";");
  int no_queries = 0;

  wxStopWatch sw;

  // Run all queries.
  while (tkz.HasMoreTokens() && !m_Stopped)
  {
    wxString query = tkz.GetNextToken();
    query.Trim(true);
    query.Trim(false);

    if (!query.empty())
    {
      try
      {
        RunQuery(query, no_queries == 0);
        no_queries++;
        wxYield();
      }
      catch (otl_exception& p)
      {
        m_Statistics.Inc(_("Number of query errors"));
        m_Shell->AppendText(_("\nerror: ") +  Quoted(p.msg) + _(" in: ") + Quoted(query));
      }
    }
  }

  m_Shell->Prompt(wxString::Format(_("\n%d queries (%.3f seconds)"),
    no_queries,
    (float)sw.Time() / (float)1000));
}

void MyFrame::UpdateStatistics(const wxStopWatch& sw, long rpc)
{
  m_Shell->AppendText(wxString::Format(_("\n%d rows processed (%.3f seconds)"),
    rpc,
    (float)sw.Time() / (float)1000));

  m_Statistics.Set(_("Rows processed"), rpc);
  m_Statistics.Set(_("Query runtime"), sw.Time());

  m_Statistics.Inc(_("Total number of queries run"));
  m_Statistics.Inc(_("Total query runtime"), sw.Time());
  m_Statistics.Inc(_("Total rows processed"), rpc);
}
