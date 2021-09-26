////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT

#include <wex/common/cmdline.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/factory/lexers.h>
#include <wex/ui/frame.h>
#include <wx/timer.h>

#include "test.h"

const wex::path wex::test::get_path(const std::string& file, path::log_t t)
{
  return wex::test::app::get_path(file, t);
}

wex::path wex::test::app::get_path(const std::string& file, path::log_t t)
{
  return file.empty() ? m_path : path(m_path, file, t);
}

bool wex::test::app::OnInit()
{
  SetAppName("wex-test"); // as in CMakeLists

  if (!wex::app::OnInit())
  {
    return false;
  }

  m_path = path(path::current()).data().parent_path();
  m_path.append(wex::path("test")).append(wex::path("data"));
  path::current(m_path.data());

  if (!m_path.dir_exists())
  {
    log("no such dir") << m_path;
    return false;
  }

  lexers::get();

  config(_("stc.vi mode")).set(true);
  config(_("stc.Auto complete")).set(true);
  config(_("locale")).set(get_locale().GetName().ToStdString()); // for coverage

  auto* frame = new wxFrame(nullptr, -1, "test");
  frame->Show();

  return true;
}

int wex::test::app::OnRun()
{
  const int id_start    = wxWindow::NewControlId();
  auto*     timer_start = new wxTimer(this, id_start);

  timer_start->StartOnce(1000);

  Bind(
    wxEVT_TIMER,
    [=, this](wxTimerEvent& event)
    {
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

std::vector<std::pair<std::string, std::string>> get_abbreviations()
{
  return std::vector<std::pair<std::string, std::string>>{
    {"XX", "GREAT"}, // see also test-source.txt
    {"YY", "WHITE"},
    {"ZZ", "SHARK"}};
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
