////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT
#define ELPP_NO_CHECK_MACROS

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "test.h"
#include <wex/config.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/managedframe.h>
#include <wx/timer.h>

const std::string wex::test::add_pane(wex::managed_frame* frame, wxWindow* pane)
{
  static int no = 0;

  wxAuiPaneInfo info(
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

  m_path = path(path::current()).data().parent_path();
  m_path.append("src").append("lib-test").append("data");
  path::current(m_path.string());

  if (!wex::app::OnInit() || !m_path.dir_exists())
  {
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
  auto* timer = new wxTimer(this);
  timer->StartOnce(1000);

  Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    m_context->run();
    config("AllowSync").set(false);

    if (m_context->shouldExit())
    {
      OnExit();
      ExitMainLoop();
    }
  });

  return wex::app::OnRun();
}

void wex::test::app::set_context(doctest::Context* context)
{
  m_context = context;
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

wex::managed_frame* frame()
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

void process(const std::string& str, wex::shell* shell)
{
  for (unsigned i = 0; i < str.length(); ++i)
  {
    shell->process_char(str.at(i));
  }
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

    return app->OnInit() && app->OnRun();
  }
  catch (const std::exception& e)
  {
    log(e) << app->GetAppName().ToStdString();
    exit(EXIT_FAILURE);
  }
}
