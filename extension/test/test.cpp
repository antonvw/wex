////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT
#define ELPP_NO_CHECK_MACROS

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <functional>
#include <wx/timer.h>
#include <wex/config.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/process.h>
#include <wex/util.h>
#include "test.h"

void AddExtension(wex::path& fn)
{
  const auto v(fn.paths());
  const auto it = std::find(v.begin(), v.end(), "wxExtension");
  
  fn = wex::path();
  
  // If wxExtension is present, copy all subdirectories.
  if (it != v.end())
  {
    for (auto i = v.begin(); i != it; i++)
    {
      fn.Append(*i);
    }

    fn.Append("wxExtension").Append("extension");
  }
  else
  {
    for (const auto& it : v)
    {
      if (it == "build" || it == "Release" || 
          it == "Debug" || it == "Coverage")
      {
        fn.Append("extension");
        break;      
      }
    
      fn.Append(it);
    
      if (it == "extension")
      {
        break;
      }
    }
  }
}
    
const std::string AddPane(wex::managed_frame* frame, wxWindow* pane)
{
  static int no = 0;
  
  wxAuiPaneInfo info(frame->GetManager().GetAllPanes().GetCount() == 5 ?
    wxAuiPaneInfo().Center():
    wxAuiPaneInfo().Bottom());

  const std::string name("PANE " + std::to_string(no++));
  
  frame->GetManager().AddPane(pane, wxAuiPaneInfo(info)
    .Name(name)
    .MinSize(250, 200)
    .Caption(name));
  
  frame->GetManager().Update();
  
  return name;
}

const wex::path GetTestPath(const std::string& file) 
{
  return wex::test_app::GetTestPath(file);
}

wex::path wex::test_app::GetTestPath(const std::string& file)
{
  return file.empty() ?
    m_TestPath:
    path(m_TestPath.Path().string(), file);
}

bool wex::test_app::OnInit()
{
  SetAppName("wex-test"); // as in CMakeLists
  SetTestPath();
  lexers::Get();
  
  if (!app::OnInit())
  {
    return false;
  }
  
  wex::config(_("vi mode")).set(true);
  wex::config(_("locale")).set(GetLocale().GetName().ToStdString()); // for coverage
  
  return true;
}

int wex::test_app::OnRun()
{
  wxTimer* timer = new wxTimer(this);
  timer->StartOnce(1000);

  Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    m_Context->run();
    wex::config("AllowSync").set(false);
    process::KillAll();

    if (m_Context->shouldExit())
    {
      OnExit();
      ExitMainLoop();
    }});

  return app::OnRun();
}

void wex::test_app::SetContext(doctest::Context* context)
{
  m_Context = context;
}
  
void wex::test_app::SetTestPath()
{
  m_TestPath = path(path::Current(), "");
  auto v(m_TestPath.paths());
  
  if (std::find(v.begin(), v.end(), "wxExtension") == v.end())
  {
    if (std::find(v.begin(), v.end(), "extension") == v.end())
    {
      m_TestPath.replace_filename("extension");
    }
    else
    {
      AddExtension(m_TestPath);
    }
  }
  else
  {
    AddExtension(m_TestPath);
  }

  v = m_TestPath.paths();

  if (std::find(v.begin(), v.end(), "test") == v.end())
  {
    m_TestPath.Append("test").Append("data");
  }

  wex::path::Current(m_TestPath.Path().string());
}

int wex::testmain(int argc, char* argv[], wex::test_app* app)
{
  try
  {
    wxApp::SetInstance(app);
    wxEntryStart(argc, argv);

    doctest::Context context;
    context.setOption("exit", true);
    context.applyCommandLine(argc, argv);
    app->SetContext(&context);

    return app->OnInit() && app->OnRun();
  }
  catch (const std::exception& e)
  {
    wex::log(e) << "app";
    exit(EXIT_FAILURE);
  }
}
