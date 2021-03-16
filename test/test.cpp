////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "test.h"
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/frame.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/process.h>
#include <wex/shell.h>
#include <wx/timer.h>

const std::string wex::test::add_pane(wex::frame* frame, wxWindow* pane)
{
  static int no = 0;

  const auto& info(
    frame->panes() == 5 ? wxAuiPaneInfo().Center() : wxAuiPaneInfo().Bottom());

  const std::string name("PANE " + std::to_string(no++));

  frame->pane_add(
    {{pane, wxAuiPaneInfo(info).Name(name).MinSize(250, 200).Caption(name)}});

  return name;
}

const wex::path wex::test::get_path(const std::string& file)
{
  return wex::test::app::get_path(file);
}

wex::path wex::test::app::get_path(const std::string& file)
{
  return file.empty() ? m_path : path(m_path.string(), file);
}

bool wex::test::app::OnInit()
{
  SetAppName("wex-test"); // as in CMakeLists

  if (!wex::app::OnInit())
  {
    return false;
  }

  m_path = path(path::current()).data().parent_path();
  m_path.append("test").append("data");
  path::current(m_path.string());

  if (!m_path.dir_exists())
  {
    log("no such dir") << m_path;
    return false;
  }

  lexers::get();

  config(_("stc.vi mode")).set(true);
  config(_("stc.Auto complete")).set(true);
  config(_("locale")).set(get_locale().GetName().ToStdString()); // for coverage

  return true;
}

int wex::test::app::OnRun()
{
  const int id_start    = wxWindow::NewControlId();
  auto*     timer_start = new wxTimer(this, id_start);

  timer_start->StartOnce(1000);

  Bind(
    wxEVT_TIMER,
    [=, this](wxTimerEvent& event) {
      m_context->run();

      config("AllowSync").set(false);

      if (m_context->shouldExit())
      {
        remove("test-ex.txt");
        Exit();
      }
    },
    id_start);

  return wex::app::OnRun();
}

void wex::test::app::set_context(doctest::Context* context)
{
  m_context = context;
}

bool wex::test::gui_app::OnInit()
{
  if (!test::app::OnInit())
  {
    return false;
  }

  m_frame     = new wex::frame();
  m_statusbar = m_frame->setup_statusbar(
    {{"Pane0"}, // the order of panes is tested
     {"Pane1"},
     {"Pane2"},
     {"Pane3"},
     {"Pane4"},
     {"PaneInfo"},
     {"PaneLexer"},
     {"PaneMode"},
     {"PaneFileType"},
     {"LastPane"}});
  m_stc = new stc();
  m_frame->Show();

  process::prepare_output(m_frame);

  add_pane(m_frame, m_stc);

  return true;
}

std::vector<std::pair<std::string, std::string>> get_abbreviations()
{
  return std::vector<std::pair<std::string, std::string>>{
    {"XX", "GREAT"}, // see also test-source.txt
    {"YY", "WHITE"},
    {"ZZ", "SHARK"}};
}

std::vector<std::string> get_builtin_variables()
{
  std::vector<std::string> v;

  for (const auto i : wex::ex::get_macros().get_variables())
  {
    if (i.second.is_builtin())
    {
      v.push_back(i.first);
    }
  }

  return v;
}

wex::frame* frame()
{
  return wex::test::gui_app::frame();
}

wex::statusbar* get_statusbar()
{
  return wex::test::gui_app::get_statusbar();
}

wex::stc* get_stc()
{
  return wex::test::gui_app::get_stc();
}

int wex::test::main(int argc, char* argv[], wex::test::app* app)
{
#ifndef __WXMSW__
  signal(SIGPIPE, SIG_IGN);
#endif

  try
  {
    wxApp::SetInstance(app);
    wxEntryStart(argc, argv);

    doctest::Context context;
    context.setOption("exit", true);
    context.applyCommandLine(argc, argv);
    app->set_context(&context);

    std::string text;

    const std::string level{"-V"};

    for (int i = 0; i < argc; i++)
    {
      if (strcmp(argv[i], level.c_str()) == 0 && i + 1 < argc)
      {
        const std::string value(argv[i + 1]);
        text = " " + level + " " + value;
        break;
      }
    }

    if (wex::data::cmdline c(text); !wex::cmdline().parse(c))
    {
      return false;
    }

    return app->OnInit() && app->OnRun();
  }
  catch (const std::exception& e)
  {
    log(e) << app->GetAppName().ToStdString();
    exit(EXIT_FAILURE);
  }
}
