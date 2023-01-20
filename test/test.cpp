////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT

// Outcomment this macro to use rfw testing
#define USE_DOCTEST ON

#include <wex/core/cmdline.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wx/frame.h>
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

  config(_("stc.vi mode")).set(true);
  config(_("stc.Auto complete")).set(true);
  config(_("locale")).set(get_locale().GetName().ToStdString()); // for coverage

  auto* frame = new wxFrame(nullptr, -1, "test");
  frame->Show();

  return true;
}

int wex::test::app::OnRun()
{
  if (m_context != nullptr)
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
          Exit();
        }
      },
      id_start);
  }

  return wex::app::OnRun();
}

void wex::test::app::set_context(doctest::Context* context)
{
  m_context = context;
}

std::vector<std::pair<std::string, std::string>> wex::test::get_abbreviations()
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

    if (!wxEntryStart(argc, argv))
    {
      std::cerr << "wxEntryStart failed, is DISPLAY set properly?\n";
      return 0;
    }

#ifdef USE_DOCTEST
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
      return 0;
    }
#else
    if (wex::data::cmdline c(argc, argv); !wex::cmdline().parse(c))
    {
      return 0;
    }
#endif

    return app->OnInit() && app->OnRun() ? 1 : 0;
  }
  catch (const std::exception& e)
  {
    log(e) << app->GetAppName().ToStdString();
    exit(EXIT_FAILURE);
  }
}
