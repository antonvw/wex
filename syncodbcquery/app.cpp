////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/config.h>
#include <wx/stockitem.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/grid.h>
#include <wx/extension/lexers.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/version.h>
#include <wx/extension/report/defs.h>
#include "app.h"

#ifndef __WXMSW__
#include "app.xpm"
#endif

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("syncodbcquery");

  if (!wex::app::OnInit())
  {
    return false;
  }

  frame* f = new frame();
  f->Show(true);

  return true;
}

frame::frame()
  : wex::history_frame()
  , m_Query(new wex::stc())
  , m_Results(new wex::grid())
  , m_Shell(new wex::shell(wex::stc_data(), "", ";"))
{
  SetIcon(wxICON(app));

  wex::menu* menuFile = new wex::menu;
  menuFile->Append(wxID_NEW);
  menuFile->Append(wxID_OPEN);
  GetFileHistory().UseMenu(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wex::menu* menuDatabase = new wex::menu;
  menuDatabase->Append(ID_DATABASE_OPEN, wex::ellipsed(_("&Open")));
  menuDatabase->Append(ID_DATABASE_CLOSE, _("&Close"));

  wex::menu* menuQuery = new wex::menu;
  menuQuery->Append(wxID_EXECUTE);
  menuQuery->Append(wxID_STOP);

#ifndef __WXOSX__
  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(wxID_PREFERENCES);
#else
  menuQuery->Append(wxID_PREFERENCES); // is moved!
#endif

  wex::menu* menuView = new wex::menu();
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
#ifndef __WXOSX__
  menubar->Append(menuOptions, _("&Options"));
#endif
  menubar->Append(menuHelp, wxGetStockLabel(wxID_HELP));
  SetMenuBar(menubar);

  m_Results->CreateGrid(0, 0);
  m_Results->EnableEditing(false); // this is a read-only grid

  m_Shell->SetFocus();

  SetupStatusBar({
    {"PaneInfo", 100, _("Lines").ToStdString()},
    {"PaneTheme", 50, _("Theme").ToStdString()}});

  if (wex::lexers::Get()->GetThemes() <= 1)
  {
    m_StatusBar->ShowField("PaneTheme", false);
  }

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
      Bottom().
      MaximizeButton(true));

  GetManager().AddPane(m_Query,
    wxAuiPaneInfo().
      Name("QUERY").
      Caption(_("Query")).
      CloseButton(true).
      MaximizeButton(true));

  GetManager().AddPane(m_Statistics.Show(this),
    wxAuiPaneInfo().Left().
      Hide().
      MaximizeButton(true).
      Caption(_("Statistics")).
      Name("STATISTICS"));

  GetManager().LoadPerspective(wxConfigBase::Get()->Read("Perspective"));
  GetManager().GetPane("QUERY").Show(false);

  GetManager().Update();
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    if (wex::file_dialog(
      &m_Query->GetFile()).ShowModalIfChanged()  != wxID_CANCEL)
    {
      wxConfigBase::Get()->Write("Perspective", GetManager().SavePerspective());
      event.Skip();
    }});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetDescription(_("This program offers a general ODBC query."));
    info.SetVersion(wex::get_version_info().Get());
    info.SetCopyright(wex::get_version_info().Copyright());
    info.AddDeveloper(wex::otl::VersionInfo().Get());
    wxAboutBox(info);
    }, wxID_ABOUT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Stopped = false;
    if (m_Query->GetText().empty()) return;
    if (m_Results->IsShown())
    {
      m_Results->ClearGrid();
    }
    // Skip sql comments.
    std::regex re("--.*$");
    std::string output = std::regex_replace(m_Query->GetText().ToStdString(), re, "", std::regex_constants::format_sed);
    // Queries are seperated by ; character.
    wex::tokenizer tkz(output, ";");
    int no_queries = 0;
    m_Running = true;
    const auto start = std::chrono::system_clock::now();
    // Run all queries.
    while (tkz.HasMoreTokens() && !m_Stopped)
    {
      std::string query = tkz.GetNextToken();
      if (!query.empty())
      {
        try
        {
          RunQuery(query, no_queries == 0);
          no_queries++;
        }
        catch (otl_exception& p)
        {
          m_Statistics.Inc("Number of query errors");
          m_Shell->AppendText(
            "\nerror: " +  wex::quoted(std::string((const char*)p.msg)) + 
            " in: " + wex::quoted(query));
        }
      }
    }
    const auto end = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
    m_Shell->Prompt(wxString::Format(_("\n%d queries (%.3f seconds)"),
      no_queries,
      (float)milli.count() / (float)1000).ToStdString());
    m_Running = false;}, wxID_EXECUTE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Query->GetFile().FileNew(wex::path());
    m_Query->SetFocus();
    ShowPane("QUERY");}, wxID_NEW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::open_files_dialog(
      this, 
      wxFD_OPEN | wxFD_CHANGE_DIR, 
      "sql files (*.sql)|*.sql|" + 
      _("All Files") + wxString::Format(" (%s)|%s",
        wxFileSelectorDefaultWildcardStr,
        wxFileSelectorDefaultWildcardStr),
      true);}, wxID_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Query->GetFile().FileSave();}, wxID_SAVE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::file_dialog dlg(
      &m_Query->GetFile(), 
      wex::window_data().
        Style(wxFD_SAVE).
        Parent(this).
        Title(wxGetStockLabel(wxID_SAVEAS).ToStdString()));
    if (dlg.ShowModal() == wxID_OK)
    {
       m_Query->GetFile().FileSave(dlg.GetPath().ToStdString());
    }}, wxID_SAVEAS);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Running = false;
    m_Stopped = true;}, wxID_STOP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_otl.Logoff())
    {
      m_Shell->SetPrompt(">");
    }}, ID_DATABASE_CLOSE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_otl.Logon())
    {
      m_Shell->SetPrompt(m_otl.Datasource() + ">");
    }}, ID_DATABASE_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_otl.IsConnected())
    {
      try
      {
        const std::string input(event.GetString().ToStdString());
        if (!input.empty())
        {
          const std::string query = input.substr(
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

        m_Shell->AppendText("\nerror: " + wex::quoted(std::string((const char*)p.msg)));
      }
    }
    else
    {
      m_Shell->AppendText("\nnot connected");
    }
    m_Shell->Prompt();}, wex::ID_SHELL_COMMAND);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Stopped = true;
    m_Shell->Prompt("cancelled");}, wex::ID_SHELL_COMMAND_STOP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("QUERY");}, ID_VIEW_QUERY);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("RESULTS");}, ID_VIEW_RESULTS);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("STATISTICS");}, ID_VIEW_STATISTICS);
  
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
    event.Enable(!GetFileHistory().GetHistoryFile().Path().empty());}, ID_RECENTFILE_MENU);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("QUERY").IsShown());}, ID_VIEW_QUERY);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("RESULTS").IsShown());}, ID_VIEW_RESULTS);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(GetManager().GetPane("STATISTICS").IsShown());}, ID_VIEW_STATISTICS);
  
  // Do automatic connect.
  if (!m_otl.Datasource().empty() && m_otl.Logon(wex::window_data().Button(0)))
  {
    m_Shell->SetPrompt(m_otl.Datasource() + ">");
  }
  else
  {
    m_Shell->SetPrompt(">");
  }
}

void frame::OnCommandItemDialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  if (dialogid == wxID_PREFERENCES)
  {
    m_Query->ConfigGet();
    m_Shell->ConfigGet();
  }
  else
  {
    wex::history_frame::OnCommandItemDialog(dialogid, event);
  }
}

wex::stc* frame::OpenFile(const wex::path& filename, const wex::stc_data& data)
{
  if (m_Query->Open(filename, data))
  {
    ShowPane("QUERY");
  }
  
  return m_Query;
}

void frame::RunQuery(const std::string& query, bool empty_results)
{
  std::string query_lower = query;
  for (auto & c : query_lower) c = ::tolower(c);
  const auto start = std::chrono::system_clock::now();

  std::chrono::milliseconds milli;
  long rpc;

  // Query functions supported by ODBC
  // $SQLTables, $SQLColumns, etc.
  // $SQLTables $1:'%'
  // allow you to get database schema.
  if (query_lower.find("select") == 0 ||
      query_lower.find("describe") == 0||
      query_lower.find("show") == 0 ||
      query_lower.find("explain") == 0 ||
      query_lower.find("$sql" == 0))
  {
    rpc = m_Results->IsShown() ? 
      m_otl.Query(query, m_Results, m_Stopped, empty_results):
      m_otl.Query(query, m_Shell, m_Stopped);
    const auto end = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    milli = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
  }
  else
  {
    rpc = m_otl.Query(query);
    const auto end = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    milli = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
  }

  m_Shell->AppendText(wxString::Format(_("\n%ld rows processed (%.3f seconds)"),
    rpc,
    (float)milli.count() / (float)1000));

  m_Statistics.Set("Rows processed", rpc);
  m_Statistics.Set("Query runtime", milli.count());

  m_Statistics.Inc("Total number of queries run");
  m_Statistics.Inc("Total query runtime", milli.count());
  m_Statistics.Inc("Total rows processed", rpc);

  m_Shell->DocumentEnd();
}

void frame::StatusBarClicked(const std::string& pane)
{
  if (pane == "PaneTheme")
  {
    if (wex::lexers::Get()->ShowThemeDialog(this))
    {
      m_Query->GetLexer().Set(m_Query->GetLexer().GetDisplayLexer());
      m_Shell->GetLexer().Set(m_Shell->GetLexer().GetDisplayLexer());

      m_StatusBar->ShowField(
        "PaneLexer", 
        !wex::lexers::Get()->GetTheme().empty());
        
      StatusText(wex::lexers::Get()->GetTheme(), "PaneTheme");
    }
  }
  else
  {
    wex::history_frame::StatusBarClicked(pane);
  }
}
