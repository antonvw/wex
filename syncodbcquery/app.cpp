////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/config.h>
#include <wx/stockitem.h>
#include <wx/tokenzr.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/grid.h>
#include <wx/extension/lexers.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/util.h>
#include "app.h"

#ifndef __WXMSW__
#include "app.xpm"
#endif

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
  SetAppName("syncodbcquery");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  Frame *frame = new Frame();
  frame->Show(true);

  return true;
}

Frame::Frame()
  : wxExFrameWithHistory(NULL, wxID_ANY, wxTheApp->GetAppDisplayName())
  , m_Running(false)
  , m_Stopped(false)
  , m_Results( new wxExGrid(this))
  , m_Query( new wxExSTC(this))
  , m_Shell( new wxExShell(this, ">", ";", true, 50))
{
  SetIcon(wxICON(app));

  wxExMenu* menuFile = new wxExMenu;
  menuFile->Append(wxID_NEW);
  menuFile->Append(wxID_OPEN);
  GetFileHistory().UseMenu(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxExMenu* menuDatabase = new wxExMenu;
  menuDatabase->Append(ID_DATABASE_OPEN, wxExEllipsed(_("&Open")));
  menuDatabase->Append(ID_DATABASE_CLOSE, _("&Close"));

  wxExMenu* menuQuery = new wxExMenu;
  menuQuery->Append(wxID_EXECUTE);
  menuQuery->Append(wxID_STOP);

  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(wxID_PREFERENCES);

  wxExMenu* menuView = new wxExMenu();
  AppendPanes(menuView);
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_VIEW_QUERY, _("Query"));
  menuView->AppendCheckItem(ID_VIEW_RESULTS, _("Results"));
  menuView->AppendCheckItem(ID_VIEW_STATISTICS, _("Statistics"));

  wxMenu* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, wxGetStockLabel(wxID_FILE));
  menubar->Append(menuView, _("&View"));
  menubar->Append(menuDatabase, _("&Connection"));
  menubar->Append(menuQuery, _("&Query"));
  menubar->Append(menuOptions, _("&Options"));
  menubar->Append(menuHelp, wxGetStockLabel(wxID_HELP));
  SetMenuBar(menubar);

  m_Results->CreateGrid(0, 0);
  m_Results->EnableEditing(false); // this is a read-only grid

  if (wxExLexers::Get()->GetCount() > 0)
  {
    m_Query->SetLexer("sql");
    m_Shell->SetLexer("sql");
  }

  m_Shell->SetFocus();

#if wxUSE_STATUSBAR
  SetupStatusBar(std::vector<wxExStatusBarPane>{
    wxExStatusBarPane(),
    wxExStatusBarPane("PaneInfo", 100, _("Lines"))});
#endif

  GetToolBar()->AddControls(false); // no realize yet
  GetToolBar()->AddTool(wxID_EXECUTE, 
    wxEmptyString,
    wxArtProvider::GetBitmap(
      wxART_GO_FORWARD, wxART_TOOLBAR, GetToolBar()->GetToolBitmapSize()),
    wxGetStockLabel(wxID_EXECUTE, wxSTOCK_NOFLAGS));
  GetToolBar()->Realize();

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

  GetManager().LoadPerspective(wxConfigBase::Get()->Read("Perspective"));
  GetManager().GetPane("QUERY").Show(false);

  GetManager().Update();
  
  Bind(wxEVT_CLOSE, [=](wxCloseEvent& event) {
    if (wxExFileDialog(this,
      &m_Query->GetFile()).ShowModalIfChanged()  != wxID_CANCEL)
    {
      wxConfigBase::Get()->Write("Perspective", GetManager().SavePerspective());
      event.Skip();
    }});
    
  EVT_MENU(wxID_ABOUT, Frame::OnCommand)
  EVT_MENU(wxID_EXECUTE, Frame::OnCommand)
  EVT_MENU(wxID_EXIT, Frame::OnCommand)
  EVT_MENU(wxID_NEW, Frame::OnCommand)
  EVT_MENU(wxID_OPEN, Frame::OnCommand)
  EVT_MENU(wxID_SAVE, Frame::OnCommand)
  EVT_MENU(wxID_SAVEAS, Frame::OnCommand)
  EVT_MENU(wxID_STOP, Frame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND, Frame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND_STOP, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, Frame::OnCommand)
  EVT_MENU_RANGE(ID_FIRST, ID_LAST, Frame::OnCommand)

  void OnCommand(wxCommandEvent& event);
void Frame::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetDescription(_("This program offers a general ODBC query."));
    info.SetVersion(wxExGetVersionInfo().GetVersionOnlyString());
    info.SetCopyright(wxExGetVersionInfo().GetCopyright());
    info.AddDeveloper(wxExOTL::VersionInfo().GetVersionString());
    wxAboutBox(info);
    }
    break;

  case wxID_EXECUTE:
    m_Stopped = false;
    RunQueries(m_Query->GetText());
    break;

  case wxID_EXIT:
    Close(true);
    break;

  case wxID_NEW:
    m_Query->GetFile().FileNew(wxExFileName());
    
    if (wxExLexers::Get()->GetCount() > 0)
    {
      m_Query->SetLexer("sql");
    }
    
    m_Query->SetFocus();
    ShowPane("QUERY");
    break;

  case wxID_OPEN:
    wxExOpenFilesDialog(
      this, 
      wxFD_OPEN | wxFD_CHANGE_DIR, 
      "sql files (*.sql)|*.sql", 
      true);
    break;

  case wxID_SAVE:
    m_Query->GetFile().FileSave();
    break;

  case wxID_SAVEAS:
    {
      wxExFileDialog dlg(
        this, 
        &m_Query->GetFile(), 
        wxGetStockLabel(wxID_SAVEAS), 
        wxFileSelectorDefaultWildcardStr, 
        wxFD_SAVE);

      if (dlg.ShowModal() == wxID_OK)
      {
         m_Query->GetFile().FileSave(dlg.GetPath());
      }
    }
    break;

  case wxID_STOP:
    m_Running = false;
    m_Stopped = true;
    break;

  case ID_DATABASE_CLOSE:
    if (m_otl.Logoff())
    {
      m_Shell->SetPrompt(">");
    }
    break;

  case ID_DATABASE_OPEN:
    if (m_otl.Logon(this))
    {
      m_Shell->SetPrompt(m_otl.Datasource() + ">");
    }
    break;

  case ID_SHELL_COMMAND:
    if (m_otl.IsConnected())
    {
      try
      {
        const wxString input(event.GetString());
        
        if (!input.empty())
        {
          const wxString query = input.substr(
            0,
            input.length() - 1);

          m_Stopped = false;
          RunQuery(query, true);
        }
      }
      catch (otl_exception& p)
      {
        if (m_Results->IsShown())
        {
          m_Results->EndBatch();
        }

        m_Shell->AppendText(_("\nerror: ") + wxExQuoted(p.msg));
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
    m_Shell->Prompt(_("cancelled"));
    break;

  case ID_VIEW_QUERY: TogglePane("QUERY"); break;
  case ID_VIEW_RESULTS: TogglePane("RESULTS"); break;
  case ID_VIEW_STATISTICS: TogglePane("STATISTICS"); break;

  default: 
    wxFAIL;
  }
}

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_Query->GetModify());}, wxID_SAVE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_Query->GetLength() > 0);}, wxID_SAVEAS);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_Running);}, wxID_STOP);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_otl.IsConnected());}, ID_DATABASE_CLOSE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!m_otl.IsConnected());}, ID_DATABASE_OPEN);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    // If we have a query, you can hide it, but still run it.
    event.Enable(m_Query->GetLength() > 0 && m_otl.IsConnected());}, wxID_EXECUTE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!GetFileHistory().GetHistoryFile().empty());}, ID_RECENTFILE_MENU);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("QUERY").IsShown());}, ID_VIEW_QUERY);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("RESULTS").IsShown());}, ID_VIEW_RESULTS);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("STATISTICS").IsShown());}, ID_VIEW_STATISTICS);
}

void Frame::OnCommandConfigDialog(
  wxWindowID dialogid,
  int commandid)
{
  if (dialogid == wxID_PREFERENCES)
  {
    m_Query->ConfigGet();
    m_Shell->ConfigGet();
    m_Shell->GetVi().Use(false);
  }
  else
  {
    wxExFrameWithHistory::OnCommandConfigDialog(dialogid, commandid);
  }
}

bool Frame::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  int col_number,
  long flags)
{
  if (m_Query->Open(filename, line_number, match, col_number, flags))
  {
    GetManager().GetPane("QUERY").Show(true);
    GetManager().Update();
    return true;
  }
  else
  {
    return false;
  }  
}

void Frame::RunQuery(const wxString& query, bool empty_results)
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
      rpc = m_otl.Query(query, m_Results, m_Stopped, empty_results);
    }
    else
    {
      rpc = m_otl.Query(query, m_Shell, m_Stopped);
    }

    sw.Pause();

    UpdateStatistics(sw.Time(), rpc);
  }
  else
  {
    const auto rpc = m_otl.Query(query);

    sw.Pause();

    UpdateStatistics(sw.Time(), rpc);
  }

  m_Shell->DocumentEnd();
}

void Frame::RunQueries(const wxString& text)
{
  if (text.empty())
  {
    return;
  }
  
  if (m_Results->IsShown())
  {
    m_Results->ClearGrid();
  }

  // Skip sql comments.
  std::regex re("--.*$");
  wxString output = std::regex_replace(text.ToStdString(), re, "", std::regex_constants::format_sed);

  // Queries are seperated by ; character.
  wxStringTokenizer tkz(output, ";");
  int no_queries = 0;

  wxStopWatch sw;
  m_Running = true;

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
      }
      catch (otl_exception& p)
      {
        m_Statistics.Inc(_("Number of query errors"));
        m_Shell->AppendText(
          _("\nerror: ") +  wxExQuoted(p.msg) + 
          _(" in: ") + wxExQuoted(query));
      }
    }
  }

  m_Shell->Prompt(wxString::Format(_("\n%d queries (%.3f seconds)"),
    no_queries,
    (float)sw.Time() / (float)1000));

  m_Running = false;
}

void Frame::UpdateStatistics(long time, long rpc)
{
  m_Shell->AppendText(wxString::Format(_("\n%ld rows processed (%.3f seconds)"),
    rpc,
    (float)time / (float)1000));

  m_Statistics.Set(_("Rows processed"), rpc);
  m_Statistics.Set(_("Query runtime"), time);

  m_Statistics.Inc(_("Total number of queries run"));
  m_Statistics.Inc(_("Total query runtime"), time);
  m_Statistics.Inc(_("Total rows processed"), rpc);
}
