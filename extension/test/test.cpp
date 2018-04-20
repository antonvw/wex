////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018
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
#include <wx/uiaction.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
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
    
const std::string AddPane(wxExManagedFrame* frame, wxWindow* pane)
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

const wxExPath GetTestPath(const std::string& file) 
{
  return wxExTestApp::GetTestPath(file);
}

void SystemArg(
  const std::string& cmd, const std::string& file, const std::string& dir)
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
  
  for (const auto& it : {"cht.txt", "conf.elp", "lexers.xml", "macros.xml", "menus.xml"})
  {
    SystemArg(cp, it, dir);
  }

#if wxExUSE_OTL
  (void)system(cp + " .odbc.ini " + wxGetHomeDir());
#endif

  // Create the global lexers object (after copying lexers.xml), 
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

wxExPath wxExTestApp::GetTestPath(const std::string& file)
{
  return file.empty() ?
    m_TestPath:
    wxExPath({m_TestPath.Path().string(), file});
}

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui");
  SetTestPath();
  SetEnvironment(wxExConfigDir());
  
  if (!wxExApp::OnInit())
  {
    return false;
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
    m_Context->run();
    wxConfigBase::Get()->Write("AllowSync", 0);
    wxExProcess::KillAll();

    if (m_Context->shouldExit())
    {
      OnExit();
      ExitMainLoop();
    }});

  return wxExApp::OnRun();
}

void wxExTestApp::SetContext(doctest::Context* context)
{
  m_Context = context;
}
  
void wxExTestApp::SetTestPath()
{
  m_TestPath = wxExPath(wxExPath::Current(), "");
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

  wxExPath::Current(m_TestPath.Path().string());
}

int wxExTestMain(int argc, char* argv[], wxExTestApp* app)
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
    wxExLog(e) << "app";
    exit(EXIT_FAILURE);
  }
}
