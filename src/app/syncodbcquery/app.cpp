////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "app.h"
#include <wex/bind.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/file-dialog.h>
#include <wex/grid.h>
#include <wex/lexers.h>
#include <wex/menubar.h>
#include <wex/report/defs.h>
#include <wex/shell.h>
#include <wex/statusbar.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/version.h>
#include <wx/aboutdlg.h>
#include <wx/stockitem.h>

#ifndef __WXMSW__
#include "app.xpm"
#endif

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("syncodbcquery");

  if (wex::data::cmdline data(argc, argv);
      !wex::app::OnInit() ||
      !wex::cmdline(
         {},
         {{{"datasource,d", "odbc datasource"},
           {wex::cmdline::STRING,
            [&](const std::any& a) {
              wex::config(_("Datasource")).set(std::any_cast<std::string>(a));
            }}},
          {{"password,p", "password for user"},
           {wex::cmdline::STRING,
            [&](const std::any& a) {
              wex::config(_("Password")).set(std::any_cast<std::string>(a));
            }}},
          {{"user,u", "user to login"},
           {wex::cmdline::STRING,
            [&](const std::any& a) {
              wex::config(_("User")).set(std::any_cast<std::string>(a));
            }}}})
         .parse(data))
  {
    return false;
  }

  auto* f = new frame();
  f->Show(true);

  return true;
}

frame::frame()
  : wex::report::frame()
  , m_query(new wex::stc())
  , m_results(new wex::grid())
  , m_shell(new wex::shell(wex::data::stc(), "", ";"))
{
  const auto idDatabaseClose  = wxWindow::NewControlId();
  const auto idDatabaseOpen   = wxWindow::NewControlId();
  const auto idRecentfileMenu = wxWindow::NewControlId();
  const auto idViewQuery      = wxWindow::NewControlId();
  const auto idViewResults    = wxWindow::NewControlId();
  const auto idViewStatistics = wxWindow::NewControlId();

  SetIcon(wxICON(app));

  auto* menuQuery = new wex::menu(
    {{wxID_EXECUTE, "", wex::data::menu().action([=](wxCommandEvent& event) {
        m_stopped = false;
        if (m_query->get_text().empty())
          return;
        if (m_results->IsShown())
        {
          m_results->ClearGrid();
        }
        // Skip sql comments.
        std::regex  re("--.*$");
        std::string output = std::regex_replace(
          m_query->get_text(),
          re,
          "",
          std::regex_constants::format_sed);
        // Queries are separated by ; character.
        wex::tokenizer tkz(output, ";");
        int            no_queries = 0;
        m_running                 = true;
        const auto start          = std::chrono::system_clock::now();
        // Run all queries.
        while (tkz.has_more_tokens() && !m_stopped)
        {
          std::string query = tkz.get_next_token();
          if (!query.empty())
          {
            try
            {
              run_query(query, no_queries == 0);
              no_queries++;
            }
            catch (otl_exception& p)
            {
              m_statistics.inc("Number of query errors");
              m_shell->AppendText(
                "\nerror: " + wex::quoted(std::string((const char*)p.msg)) +
                " in: " + wex::quoted(query));
            }
          }
        }
        const auto end     = std::chrono::system_clock::now();
        const auto elapsed = end - start;
        const auto milli =
          std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        m_shell->prompt(wxString::Format(
                          _("\n%d queries (%.3f seconds)"),
                          no_queries,
                          (float)milli.count() / (float)1000)
                          .ToStdString());
        m_running = false;
      })},
     {wxID_STOP, "", wex::data::menu().action([=](wxCommandEvent& event) {
        m_running = false;
        m_stopped = true;
      })}});

#ifndef __WXOSX__
  auto* menuOptions = new wex::menu({{wxID_PREFERENCES}});
#else
  menuQuery->append({{wxID_PREFERENCES}}); // is moved!
#endif

  SetMenuBar(new wex::menubar(
    {{new wex::menu(
        {{wxID_NEW, "", wex::data::menu().action([=](wxCommandEvent& event) {
            m_query->get_file().file_new(wex::path());
            m_query->SetFocus();
            pane_show("QUERY");
          })},
         {wxID_OPEN, "", wex::data::menu().action([=](wxCommandEvent& event) {
            wex::data::window data;
            data.style(wxFD_OPEN | wxFD_CHANGE_DIR)
              .wildcard(
                "sql files (*.sql)|*.sql|" + _("All Files") +
                wxString::Format(
                  " (%s)|%s",
                  wxFileSelectorDefaultWildcardStr,
                  wxFileSelectorDefaultWildcardStr));
            wex::open_files_dialog(this, true, wex::data::stc(data));
          })},
         {idRecentfileMenu, file_history()},
         {},
         {wxID_SAVE,
          "",
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              m_query->get_file().file_save();
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Enable(m_query->GetModify());
            })},
         {wxID_SAVEAS,
          "",
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              wex::file_dialog dlg(
                &m_query->get_file(),
                wex::data::window().style(wxFD_SAVE).parent(this).title(
                  wxGetStockLabel(wxID_SAVEAS, wxSTOCK_NOFLAGS).ToStdString()));
              if (dlg.ShowModal() == wxID_OK)
              {
                m_query->get_file().file_save(dlg.GetPath().ToStdString());
              }
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Enable(m_query->GetLength() > 0);
            })},
         {},
         {wex::menu_item::EXIT}}),
      wxGetStockLabel(wxID_FILE)},
     {new wex::menu(
        {{this},
         {},
         {idViewQuery,
          _("Query"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              pane_toggle("QUERY");
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(pane_is_shown("QUERY"));
            })},
         {idViewResults,
          _("Results"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              pane_toggle("RESULTS");
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(pane_is_shown("RESULTS"));
            })},
         {idViewStatistics,
          _("Statistics"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              pane_toggle("STATISTICS");
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(pane_is_shown("STATISTICS"));
            })}}),
      _("&View")},
     {new wex::menu(
        {{idDatabaseOpen,
          wex::ellipsed(_("&Open")),
          wex::data::menu().action([=](wxCommandEvent& event) {
            if (m_otl.logon())
            {
              m_shell->set_prompt(m_otl.datasource() + ">");
            }
          })},
         {idDatabaseClose,
          _("&Close"),
          wex::data::menu().action([=](wxCommandEvent& event) {
            if (m_otl.logoff())
            {
              m_shell->set_prompt(">");
            }
          })}}),
      _("&Connection")},
     {menuQuery, _("&Query")},
#ifndef __WXOSX__
     {menuOptions, _("&Options")},
#endif
     {new wex::menu(
        {{wxID_ABOUT, "", wex::data::menu().action([=](wxCommandEvent& event) {
            wxAboutDialogInfo info;
            info.SetIcon(GetIcon());
            info.SetDescription(_("This program offers a general ODBC query."));
            info.SetVersion(wex::get_version_info().get());
            info.SetCopyright(wex::get_version_info().copyright());
            info.AddDeveloper("otl:" + wex::otl::get_version_info().get());
            wxAboutBox(info);
          })}}),
      wxGetStockLabel(wxID_HELP)}}));

  m_results->CreateGrid(0, 0);
  m_results->EnableEditing(false); // this is a read-only grid

  m_shell->SetFocus();

  setup_statusbar(
    {wex::statusbar_pane("PaneInfo", 100).help(_("Lines")), {"PaneTheme", 50}});

  if (wex::lexers::get()->get_themes_size() <= 1)
  {
    m_statusbar->pane_show("PaneTheme", false);
  }

  get_toolbar()->add_standard(false); // no realize yet
  get_toolbar()->add_tool(
    {wex::data::toolbar_item(wxID_EXECUTE)
       .bitmap(wxArtProvider::GetBitmap(
         wxART_GO_FORWARD,
         wxART_TOOLBAR,
         get_toolbar()->GetToolBitmapSize()))
       .label(wxGetStockLabel(wxID_EXECUTE, wxSTOCK_NOFLAGS))});

  pane_add(
    {{m_shell, wxAuiPaneInfo().Name("CONSOLE").CenterPane()},
     {m_results,
      wxAuiPaneInfo()
        .Name("RESULTS")
        .Caption(_("Results"))
        .CloseButton(true)
        .Bottom()
        .MaximizeButton(true)},
     {m_query,
      wxAuiPaneInfo()
        .Name("QUERY")
        .Caption(_("Query"))
        .CloseButton(true)
        .MaximizeButton(true)},
     {m_statistics.show(this),
      wxAuiPaneInfo()
        .Left()
        .Hide()
        .MaximizeButton(true)
        .Caption(_("Statistics"))
        .Name("STATISTICS")}},
    "Perspective");

  pane_show("QUERY", false);

  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    if (
      wex::file_dialog(&m_query->get_file()).show_modal_if_changed() !=
      wxID_CANCEL)
    {
      event.Skip();
    }
  });

  wex::bind(this).command(
    {{[=](wxCommandEvent& event) {
        if (m_otl.is_connected())
        {
          try
          {
            const std::string input(event.GetString().ToStdString());
            if (!input.empty())
            {
              const std::string query = input.substr(0, input.length() - 1);

              m_stopped = false;
              run_query(query, true);
            }
          }
          catch (otl_exception& p)
          {
            if (m_results->IsShown())
            {
              m_results->EndBatch();
            }

            m_shell->AppendText(
              "\nerror: " + wex::quoted(std::string((const char*)p.msg)));
          }
        }
        else
        {
          m_shell->AppendText("\nnot connected");
        }
        m_shell->prompt();
      },
      wex::ID_SHELL_COMMAND},
     {[=](wxCommandEvent& event) {
        m_stopped = true;
        m_shell->prompt("cancelled");
      },
      wex::ID_SHELL_COMMAND_STOP}});

  wex::bind(this).ui(
    {{[=](wxUpdateUIEvent& event) {
        event.Enable(m_running);
      },
      wxID_STOP},
     {[=](wxUpdateUIEvent& event) {
        event.Enable(m_otl.is_connected());
      },
      idDatabaseClose},
     {[=](wxUpdateUIEvent& event) {
        event.Enable(!m_otl.is_connected());
      },
      idDatabaseOpen},
     {[=](wxUpdateUIEvent& event) {
        // If we have a query, you can hide it, but still run it.
        event.Enable(m_query->GetLength() > 0 && m_otl.is_connected());
      },
      wxID_EXECUTE},
     {[=](wxUpdateUIEvent& event) {
        event.Enable(!file_history().get_history_file().empty());
      },
      idRecentfileMenu}});

  // Do automatic connect.
  if (!m_otl.datasource().empty() && m_otl.logon(wex::data::window().button(0)))
  {
    m_shell->set_prompt(m_otl.datasource() + ">");
  }
  else
  {
    m_shell->set_prompt(">");
  }
}

void frame::on_command_item_dialog(
  wxWindowID            dialogid,
  const wxCommandEvent& event)
{
  if (dialogid == wxID_PREFERENCES)
  {
    m_query->config_get();
    m_shell->config_get();
  }
  else
  {
    wex::report::frame::on_command_item_dialog(dialogid, event);
  }
}

wex::stc*
frame::open_file(const wex::path& filename, const wex::data::stc& data)
{
  if (m_query->open(filename, data))
  {
    pane_show("QUERY");
  }

  return m_query;
}

void frame::run_query(const std::string& query, bool empty_results)
{
  std::string query_lower = query;
  for (auto& c : query_lower)
    c = ::tolower(c);
  const auto start = std::chrono::system_clock::now();

  std::chrono::milliseconds milli;
  long                      rpc;

  // Query functions supported by ODBC
  // $SQLTables, $SQLColumns, etc.
  // $SQLTables $1:'%'
  // allow you to get database schema.
  if (
    query_lower.find("select") == 0 || query_lower.find("describe") == 0 ||
    query_lower.find("show") == 0 || query_lower.find("explain") == 0 ||
    query_lower.find("$sql" == 0))
  {
    rpc = m_results->IsShown() ?
            m_otl.query(query, m_results, m_stopped, empty_results) :
            m_otl.query(query, m_shell, m_stopped);
    const auto end     = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    milli = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
  }
  else
  {
    rpc                = m_otl.query(query);
    const auto end     = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    milli = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
  }

  m_shell->AppendText(wxString::Format(
    _("\n%ld rows processed (%.3f seconds)"),
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
      m_query->get_lexer().set(m_query->get_lexer().display_lexer());
      m_shell->get_lexer().set(m_shell->get_lexer().display_lexer());

      m_statusbar->pane_show("PaneLexer", !wex::lexers::get()->theme().empty());

      statustext(wex::lexers::get()->theme(), "PaneTheme");
    }
  }
  else
  {
    wex::report::frame::statusbar_clicked(pane);
  }
}
