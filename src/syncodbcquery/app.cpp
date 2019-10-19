////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/stockitem.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/filedlg.h>
#include <wex/grid.h>
#include <wex/lexers.h>
#include <wex/shell.h>
#include <wex/stc.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/tokenizer.h>
#include <wex/version.h>
#include <wex/report/defs.h>
#include "app.h"

#ifndef __WXMSW__
#include "app.xpm"
#endif

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("syncodbcquery");

  if (
    !wex::app::OnInit() ||
    !wex::cmdline(
     {},
     {{{"datasource,d", "odbc datasource"}, {wex::cmdline::STRING, [&](const std::any& a) {
        wex::config(_("Datasource")).set(std::any_cast<std::string>(a));}}},
      {{"password,p", "password for user"}, {wex::cmdline::STRING, [&](const std::any& a) {
        wex::config(_("Password")).set(std::any_cast<std::string>(a));}}},
      {{"user,u", "user to login"}, {wex::cmdline::STRING, [&](const std::any& a) {
        wex::config(_("User")).set(std::any_cast<std::string>(a));}}}
     }).parse(argc, argv))
  {
    return false;
  }

  frame* f = new frame();
  f->Show(true);

  return true;
}

frame::frame()
  : wex::report::frame()
  , m_Query(new wex::stc())
  , m_Results(new wex::grid())
  , m_shell(new wex::shell(wex::stc_data(), "", ";"))
{
  const auto idDatabaseClose = wxWindow::NewControlId();
  const auto idDatabaseOpen = wxWindow::NewControlId();
  const auto idRecentfileMenu = wxWindow::NewControlId();
  const auto idViewQuery = wxWindow::NewControlId();
  const auto idViewResults = wxWindow::NewControlId();
  const auto idViewStatistics = wxWindow::NewControlId();

  SetIcon(wxICON(app));

  wex::menu* menuFile = new wex::menu;
  menuFile->append(wxID_NEW);
  menuFile->append(wxID_OPEN);
  file_history().use_menu(idRecentfileMenu, menuFile);
  menuFile->append_separator();
  menuFile->append(wxID_SAVE);
  menuFile->append(wxID_SAVEAS);
  menuFile->append_separator();
  menuFile->append(wxID_EXIT);

  wex::menu* menuDatabase = new wex::menu;
  menuDatabase->append(idDatabaseOpen, wex::ellipsed(_("&Open")));
  menuDatabase->append(idDatabaseClose, _("&Close"));

  wex::menu* menuQuery = new wex::menu;
  menuQuery->append(wxID_EXECUTE);
  menuQuery->append(wxID_STOP);

#ifndef __WXOSX__
  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(wxID_PREFERENCES);
#else
  menuQuery->append(wxID_PREFERENCES); // is moved!
#endif

  wex::menu* menuView = new wex::menu();
  append_panes(menuView);
  menuView->append_separator();
  menuView->AppendCheckItem(idViewQuery, _("Query"));
  menuView->AppendCheckItem(idViewResults, _("Results"));
  menuView->AppendCheckItem(idViewStatistics, _("Statistics"));

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

  m_shell->SetFocus();

  setup_statusbar({
    {"PaneInfo", 100, _("Lines").ToStdString()},
    {"PaneTheme", 50, _("Theme").ToStdString()}});

  if (wex::lexers::get()->get_themes_size() <= 1)
  {
    m_statusbar->show_field("PaneTheme", false);
  }

  get_toolbar()->add_controls(false); // no realize yet
  get_toolbar()->add_tool(wxID_EXECUTE, 
    std::string(),
    wxArtProvider::GetBitmap(
      wxART_GO_FORWARD, wxART_TOOLBAR, get_toolbar()->GetToolBitmapSize()),
    wxGetStockLabel(wxID_EXECUTE, wxSTOCK_NOFLAGS));
  get_toolbar()->Realize();

  manager().AddPane(m_shell,
    wxAuiPaneInfo().
      Name("CONSOLE").
      CenterPane());

  manager().AddPane(m_Results,
    wxAuiPaneInfo().
      Name("RESULTS").
      Caption(_("Results")).
      CloseButton(true).
      Bottom().
      MaximizeButton(true));

  manager().AddPane(m_Query,
    wxAuiPaneInfo().
      Name("QUERY").
      Caption(_("Query")).
      CloseButton(true).
      MaximizeButton(true));

  manager().AddPane(m_statistics.show(this),
    wxAuiPaneInfo().Left().
      Hide().
      MaximizeButton(true).
      Caption(_("Statistics")).
      Name("STATISTICS"));

  manager().LoadPerspective(wex::config("Perspective").get());
  manager().GetPane("QUERY").Show(false);

  manager().Update();
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    if (wex::file_dialog(
      &m_Query->get_file()).show_modal_if_changed()  != wxID_CANCEL)
    {
      wex::config("Perspective").set(manager().SavePerspective().ToStdString());
      event.Skip();
    }});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetDescription(_("This program offers a general ODBC query."));
    info.SetVersion(wex::get_version_info().get());
    info.SetCopyright(wex::get_version_info().copyright());
    info.AddDeveloper("otl:" + wex::otl::get_version_info().get());
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
    while (tkz.has_more_tokens() && !m_Stopped)
    {
      std::string query = tkz.get_next_token();
      if (!query.empty())
      {
        try
        {
          RunQuery(query, no_queries == 0);
          no_queries++;
        }
        catch (otl_exception& p)
        {
          m_statistics.inc("Number of query errors");
          m_shell->AppendText(
            "\nerror: " +  wex::quoted(std::string((const char*)p.msg)) + 
            " in: " + wex::quoted(query));
        }
      }
    }
    const auto end = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
    m_shell->prompt(wxString::Format(_("\n%d queries (%.3f seconds)"),
      no_queries,
      (float)milli.count() / (float)1000).ToStdString());
    m_Running = false;}, wxID_EXECUTE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Query->get_file().file_new(wex::path());
    m_Query->SetFocus();
    show_pane("QUERY");}, wxID_NEW);

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
    m_Query->get_file().file_save();}, wxID_SAVE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::file_dialog dlg(
      &m_Query->get_file(), 
      wex::window_data().
        style(wxFD_SAVE).
        parent(this).
        title(wxGetStockLabel(wxID_SAVEAS).ToStdString()));
    if (dlg.ShowModal() == wxID_OK)
    {
       m_Query->get_file().file_save(dlg.GetPath().ToStdString());
    }}, wxID_SAVEAS);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Running = false;
    m_Stopped = true;}, wxID_STOP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_otl.logoff())
    {
      m_shell->set_prompt(">");
    }}, idDatabaseClose);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_otl.logon())
    {
      m_shell->set_prompt(m_otl.datasource() + ">");
    }}, idDatabaseOpen);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_otl.is_connected())
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

        m_shell->AppendText("\nerror: " + wex::quoted(std::string((const char*)p.msg)));
      }
    }
    else
    {
      m_shell->AppendText("\nnot connected");
    }
    m_shell->prompt();}, wex::ID_SHELL_COMMAND);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Stopped = true;
    m_shell->prompt("cancelled");}, wex::ID_SHELL_COMMAND_STOP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    toggle_pane("QUERY");}, idViewQuery);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    toggle_pane("RESULTS");}, idViewResults);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    toggle_pane("STATISTICS");}, idViewStatistics);
  
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_Query->GetModify());}, wxID_SAVE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_Query->GetLength() > 0);}, wxID_SAVEAS);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_Running);}, wxID_STOP);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_otl.is_connected());}, idDatabaseClose);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!m_otl.is_connected());}, idDatabaseOpen);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    // If we have a query, you can hide it, but still run it.
    event.Enable(m_Query->GetLength() > 0 && m_otl.is_connected());}, wxID_EXECUTE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!file_history().get_history_file().empty());}, idRecentfileMenu);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(manager().GetPane("QUERY").IsShown());}, idViewQuery);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(manager().GetPane("RESULTS").IsShown());}, idViewResults);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(manager().GetPane("STATISTICS").IsShown());}, idViewStatistics);
  
  // Do automatic connect.
  if (!m_otl.datasource().empty() && m_otl.logon(wex::window_data().button(0)))
  {
    m_shell->set_prompt(m_otl.datasource() + ">");
  }
  else
  {
    m_shell->set_prompt(">");
  }
}

void frame::on_command_item_dialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  if (dialogid == wxID_PREFERENCES)
  {
    m_Query->config_get();
    m_shell->config_get();
  }
  else
  {
    wex::report::frame::on_command_item_dialog(dialogid, event);
  }
}

wex::stc* frame::open_file(const wex::path& filename, const wex::stc_data& data)
{
  if (m_Query->open(filename, data))
  {
    show_pane("QUERY");
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
      m_otl.query(query, m_Results, m_Stopped, empty_results):
      m_otl.query(query, m_shell, m_Stopped);
    const auto end = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    milli = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
  }
  else
  {
    rpc = m_otl.query(query);
    const auto end = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    milli = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
  }

  m_shell->AppendText(wxString::Format(_("\n%ld rows processed (%.3f seconds)"),
    rpc,
    (float)milli.count() / (float)1000));

  m_statistics.set("Rows processed", rpc);
  m_statistics.set("Query runtime", milli.count());

  m_statistics.inc("Total number of queries run");
  m_statistics.inc("Total query runtime", milli.count());
  m_statistics.inc("Total rows processed", rpc);

  m_shell->DocumentEnd();
}

void frame::statusbar_clicked(const std::string& pane)
{
  if (pane == "PaneTheme")
  {
    if (wex::lexers::get()->show_theme_dialog(this))
    {
      m_Query->get_lexer().set(m_Query->get_lexer().display_lexer());
      m_shell->get_lexer().set(m_shell->get_lexer().display_lexer());

      m_statusbar->show_field(
        "PaneLexer", 
        !wex::lexers::get()->theme().empty());
        
      statustext(wex::lexers::get()->theme(), "PaneTheme");
    }
  }
  else
  {
    wex::report::frame::statusbar_clicked(pane);
  }
}
