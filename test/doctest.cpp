////////////////////////////////////////////////////////////////////////////////
// Name:      doctest.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT

#include <wex/core/cmdline.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wx/timer.h>
#include <wx/window.h>

#include "doctest.h"

wex::path wex::test::doctester::get_path(const std::string& file, path::log_t t)
{
  return file.empty() ? m_path : path(m_path, file, t);
}

bool wex::test::doctester::on_init(wex::app* app)
{
  app->SetAppName("wex-test"); // as in CMakeLists

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
  config(_("locale"))
    .set(app->get_locale().GetName().ToStdString()); // for coverage

  return true;
}

void wex::test::doctester::on_run(wex::app* app)
{
  if (m_context != nullptr)
  {
    const int id_start    = wxWindow::NewControlId();
    auto*     timer_start = new wxTimer(app, id_start);

    timer_start->StartOnce(1000);

    app->Bind(
      wxEVT_TIMER,
      [=, this](wxTimerEvent& event)
      {
        m_context->run();

        config("AllowSync").set(false);

        if (m_context->shouldExit())
        {
          delete m_context;
          wxExit();
        }
      },
      id_start);
  }
}

bool wex::test::doctester::use_context(wex::app* app, int argc, char* argv[])
{
  wxApp::SetInstance(app);

  if (!wxEntryStart(argc, argv))
  {
    std::cerr << "wxEntryStart failed, is DISPLAY set properly?\n";
    return false;
  }

  // Set this variable to use rfw testing
  if (const bool use_rfw = false; use_rfw)
  {
    if (wex::data::cmdline c(argc, argv); !wex::cmdline().parse(c))
    {
      return false;
    }
  }

  m_context = new doctest::Context;

  m_context->setOption("exit", true);
  m_context->applyCommandLine(argc, argv);

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

  return true;
}
