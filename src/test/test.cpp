////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/frame.h>

#include <wex/core/log.h>
#include <wex/test/test.h>

const wex::path wex::test::get_path(const std::string& file, path::log_t t)
{
  return wex::test::doctester::get_path(file, t);
}

bool wex::test::app::OnInit()
{
  if (!wex::app::OnInit() || !on_init(this))
  {
    return false;
  }

  auto* frame = new wxFrame(nullptr, -1, "test");
  frame->Show();

  return true;
}

int wex::test::app::OnRun()
{
  on_run(this);

  return wex::app::OnRun();
}

wxLanguage wex::test::app::get_default_language() const
{
  return wxLANGUAGE_DUTCH;
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
    return app->use_context(app, argc, argv) && app->OnInit() && app->OnRun() ?
             1 :
             0;
  }
  catch (const std::exception& e)
  {
    log(e) << app->GetAppName().ToStdString();
    exit(EXIT_FAILURE);
  }
}
