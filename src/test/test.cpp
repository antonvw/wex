////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT
#define ELPP_NO_CHECK_MACROS

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/timer.h>
#include <wex/config.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include "test.h"

const std::string wex::test::add_pane(wex::managed_frame* frame, wxWindow* pane)
{
  static int no = 0;
  
  wxAuiPaneInfo info(frame->manager().GetAllPanes().GetCount() == 5 ?
    wxAuiPaneInfo().Center():
    wxAuiPaneInfo().Bottom());

  const std::string name("PANE " + std::to_string(no++));
  
  frame->manager().AddPane(pane, wxAuiPaneInfo(info)
    .Name(name)
    .MinSize(250, 200)
    .Caption(name));
  
  frame->manager().Update();
  
  return name;
}

const wex::path wex::test::get_path(const std::string& file) 
{
  return wex::test::app::get_path(file);
}

wex::path wex::test::app::get_path(const std::string& file)
{
  return file.empty() ?
    m_path:
    path(m_path.string(), file);
}

bool wex::test::app::OnInit()
{
  SetAppName("wex-test"); // as in CMakeLists

  m_path = path(path::current()).data().parent_path();
  m_path.append("src").append("test").append("data");
  path::current(m_path.string());

  if (!wex::app::OnInit() || !m_path.dir_exists())
  {
    return false;
  }

  lexers::get();
  
  config(_("vi mode")).set(true);
  config(_("Auto complete")).set(true);
  config(_("locale")).set(get_locale().GetName().ToStdString()); // for coverage
  
  return true;
}

int wex::test::app::OnRun()
{
  wxTimer* timer = new wxTimer(this);
  timer->StartOnce(1000);

  Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    m_context->run();
    config("AllowSync").set(false);

    if (m_context->shouldExit())
    {
      OnExit();
      ExitMainLoop();
    }});

  return wex::app::OnRun();
}

void wex::test::app::set_context(doctest::Context* context)
{
  m_context = context;
}
  
int wex::test::main(int argc, char* argv[], wex::test::app* app)
{
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
