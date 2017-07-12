////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017
////////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <functional>
#include <wx/config.h>
#include <wx/log.h>
#include <wx/timer.h>
#include <wx/uiaction.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/util.h>
#include "test.h"

void AddExtension(wxExPath& fn)
{
  const auto v(fn.GetPaths());
  const auto it = std::find(v.begin(), v.end(), "wxExtension");
  
  fn = wxExPath();
  
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
    
void AddPane(wxExManagedFrame* frame, wxWindow* pane)
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
}

const std::string GetTestDir()
{
  return wxExTestApp::GetTestPath().Path().string() + "/";
}
  
const wxExPath GetTestFile()
{
  return wxExPath({GetTestDir(), "test.h"});
}
  
void SystemArg(
  const std::string cmd, const std::string& file, const std::string& dir)
{
  const wxExPath path({"..", "..", "data", file});
  const std::string line(cmd + " " + path.Path().string() + " " + dir);

  (void)system(line.c_str());
}

void SetEnvironment(const std::string& dir)
{
  if (!wxExPath(dir).DirExists())
  {
    (void)system(std::string("mkdir -p " + dir).c_str());
  }

#ifdef __UNIX__
  const std::string cp("cp");
#else
  const std::string cp("COPY");
#endif
  
  SystemArg(cp, "cht.txt", dir);
  SystemArg(cp, "lexers.xml", dir);
  SystemArg(cp, "macros.xml", dir);
  SystemArg(cp, "menus.xml", dir);

#if wxExUSE_OTL
  (void)system(cp + " .odbc.ini " + wxGetHomeDir());
#endif

  // Create the global lexers object (after copying lexers.xml), 
  // it should be present in ~/.wxex-test-gui
  // (depending on platform, configuration).
  wxExLexers::Get();
}
    
bool wxExUIAction(wxWindow* win, const std::string& action, const std::string& par)
{
#if wxCHECK_VERSION(3,1,0)
  wxUIActionSimulator sim;

  if (action.find("button") == 0)
  {
    sim.MouseMove(win->GetScreenPosition() + wxPoint(100, 100));
    
    if (par.find("left") != std::string::npos || par.find("right") != std::string::npos)
    {
      sim.MouseClick(par.find("right") != std::string::npos ? wxMOUSE_BTN_RIGHT: wxMOUSE_BTN_LEFT);
    }
  }
  else if (action.find("key") == 0)
  {
  }
  else if (action.find("toolbar") == 0)
  {
    sim.MouseMove(win->GetScreenPosition() + wxPoint(5, 5));
    sim.MouseClick(wxMOUSE_BTN_LEFT);
  }
  else
  {
    return false;
  }
  
  wxTimer* timer = new wxTimer(wxTheApp);
  timer->StartOnce(100);
  wxTheApp->Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    wxUIActionSimulator sim;
    sim.Char(WXK_RETURN);});
#endif

  return true;
}
  
wxExPath wxExTestApp::m_TestPath;

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui");
  SetTestPath();
  SetEnvironment(wxExConfigDir());
  
  if (!wxExApp::OnInit())
  {
    return false;
  }
  
  const long verbose(wxConfigBase::Get()->ReadLong("verbose", 2));
  
  switch (verbose)
  {
    case 1: wxLog::SetActiveTarget(new wxLogStderr()); break;
    case 2: new wxLogNull(); break;
  }
  
  wxConfigBase::Get()->Write(_("vi mode"), true);
  wxConfigBase::Get()->Write(_("locale"), GetLocale().GetName()); // for coverage
  
  return true;
}

int wxExTestApp::OnRun()
{
  wxTimer* timer = new wxTimer(this);
  timer->StartOnce(1000);

  Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    try
    {
      const int res = m_Context->run();
      const long auto_exit(wxConfigBase::Get()->ReadLong("auto-exit", 1));
      wxExProcess::KillAll();
      if (auto_exit)
      {
        exit(res);
      }
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << "\n";
      exit(EXIT_FAILURE);
    }});

  return wxExApp::OnRun();
}

void wxExTestApp::SetContext(doctest::Context* context)
{
  m_Context = context;
}
  
void wxExTestApp::SetTestPath()
{
  m_TestPath = wxExPath(wxGetCwd().ToStdString(), "");
  auto v(m_TestPath.GetPaths());
  
  if (std::find(v.begin(), v.end(), "wxExtension") == v.end())
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

  if (!wxSetWorkingDirectory(m_TestPath.Path().string()))
  {
    std::cout << "cannot set working dir: " << m_TestPath.Path().string() << "\n";
    exit(1);
  }
}

int wxExTestMain(int argc, char* argv[], wxExTestApp* app, bool use_eventloop)
{
  doctest::Context context;
  context.applyCommandLine(argc, argv);
  
  wxApp::SetInstance(app);
  wxEntryStart(argc, argv);
  app->OnInit();
  
  if (!use_eventloop)
  {
    try
    {
      const int res = context.run();
      if (context.shouldExit()) return res;
      app->ProcessPendingEvents();
      app->ExitMainLoop();
      wxExProcess::KillAll();
      return res;
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << "\n";
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    app->SetContext(&context);
    return app->OnRun();
  }
}
