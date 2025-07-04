////////////////////////////////////////////////////////////////////////////////
// Name:      unittest.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define CATCH_CONFIG_RUNNER

#include <iostream>

#include <wx/timer.h>
#include <wx/window.h>

#include <wex/core/cmdline.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/test/unittest.h>

wex::path wex::test::unittest::get_path(const std::string& file, path::log_t t)
{
  return file.empty() ? m_path : path(m_path, file, t);
}

bool wex::test::unittest::on_init(wex::app* app)
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

void wex::test::unittest::on_run(wex::app* app)
{
  const int id_start    = wxWindow::NewControlId();
  auto*     timer_start = new wxTimer(app, id_start);

  timer_start->StartOnce(1000);

  app->Bind(
    wxEVT_TIMER,
    [=, this](const wxTimerEvent& event)
    {
      if (m_session != nullptr)
      {
        delete timer_start;

        m_session->run();

        config("AllowSync").set(false);

        if (m_exit_after_test)
        {
          delete m_session;
          wxExit();
        }
      }
      else
      {
        std::cerr << "no context to run\n";
      }
    },
    id_start);
}

bool wex::test::unittest::start(wex::app* app, int argc, char* argv[])
{
  wxApp::SetInstance(app);

  if (!wxEntryStart(argc, argv))
  {
    std::cerr << "wxEntryStart failed, is DISPLAY set properly?\n";
    return false;
  }

  m_session = new Catch::Session(); // There must be exactly one instance

  m_session->applyCommandLine(argc, argv);

  std::string text;

  const std::string level{"-V"};
  const std::string quit{"-q"};

  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], level.c_str()) == 0 && i + 1 < argc)
    {
      const std::string value(argv[i + 1]);
      text = " " + level + " " + value;
    }
    else if (strcmp(argv[i], quit.c_str()) == 0 && i + 1 < argc)
    {
      const std::string value(argv[i + 1]);
      text = " " + quit + " " + value;

      if (std::stoi(value) == 0)
      {
        m_exit_after_test = false;
      }
    }
  }

  if (wex::data::cmdline c(text); !wex::cmdline().parse(c))
  {
    return false;
  }

  return app->OnInit() && app->OnRun();
}
