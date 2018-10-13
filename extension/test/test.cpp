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
#include <wx/config.h>
#include <wx/timer.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/util.h>
#include "test.h"

void AddExtension(wex::path& fn)
{
  const auto v(fn.GetPaths());
  const auto it = std::find(v.begin(), v.end(), "wex::tension");
  
  fn = wex::path();
  
  // If wex::tension is present, copy all subdirectories.
  if (it != v.end())
  {
    for (auto i = v.begin(); i != it; i++)
    {
      fn.Append(*i);
    }

    fn.Append("wex::tension").Append("extension");
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
    wex::path(m_TestPath.Path().string(), file);
}

bool wex::test_app::OnInit()
{
  SetAppName("wex-test"); // as in CMakeLists
  SetTestPath();
  wex::lexers::Get();
  
  if (!wex::app::OnInit())
  {
    return false;
  }
  
  wxConfigBase::Get()->Write(_("vi mode"), true);
  wxConfigBase::Get()->Write(_("locale"), GetLocale().GetName()); // for coverage
  
  return true;
}

int wex::test_app::OnRun()
{
  wxTimer* timer = new wxTimer(this);
  timer->StartOnce(1000);

  Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    m_Context->run();
    wxConfigBase::Get()->Write("AllowSync", 0);
    wex::process::KillAll();

    if (m_Context->shouldExit())
    {
      OnExit();
      ExitMainLoop();
    }});

  return wex::app::OnRun();
}

void wex::test_app::SetContext(doctest::Context* context)
{
  m_Context = context;
}
  
void wex::test_app::SetTestPath()
{
  m_TestPath = wex::path(wex::path::Current(), "");
  auto v(m_TestPath.GetPaths());
  
  if (std::find(v.begin(), v.end(), "wex::tension") == v.end())
  {
    if (std::find(v.begin(), v.end(), "extension") == v.end())
    {
      m_TestPath.ReplaceFileName("extension");
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

  v = m_TestPath.GetPaths();

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
