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
#include <wx/stdpaths.h>
#include <wx/timer.h>
#include <wx/uiaction.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/util.h>
#include "test.h"

void AddPane(wxExManagedFrame* frame, wxWindow* pane)
{
  static int no = 0;
  
  wxAuiPaneInfo info(frame->GetManager().GetAllPanes().GetCount() == 5 ?
    wxAuiPaneInfo().Center():
    wxAuiPaneInfo().Bottom());
  
  const wxString name(wxString::Format("PANE %d", no++));
  
  frame->GetManager().AddPane(pane, wxAuiPaneInfo(info)
    .Name(name)
    .MinSize(250, 200)
    .Caption(name));
  
  frame->GetManager().Update();
}

void SystemArg(
  const std::string cmd, const std::string& file, const std::string& dir)
{
  const std::string line(
    cmd +
    std::string(" ..") + std::string(1, wxFileName::GetPathSeparator()) + 
    ".." + std::string(1, wxFileName::GetPathSeparator()) + 
    "data" + std::string(1, wxFileName::GetPathSeparator()) + 
    file + " " + dir);

  (void)system(line.c_str());
}

const std::string GetTestDir()
{
  return wxExTestApp::GetTestFileName().GetFullPath().ToStdString() + 
    (char)wxFileName::GetPathSeparator();
}
  
const wxExPath GetTestFile()
{
  return GetTestDir() + "test.h";
}
  
void SetEnvironment(const std::string& dir)
{
  if (!wxDirExists(dir))
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
    
void SetFindExtension(wxFileName& fn)
{
  const wxArrayString ar(fn.GetDirs());
  const int index = ar.Index("wxExtension");
  
  fn.Assign(wxFileName::GetPathSeparator(), "");
  
  // If wxExtension is present, copy all subdirectories.
  if (index != wxNOT_FOUND)
  {
    for (int i = 0; i <= index; i++)
    {
      fn.AppendDir(ar[i]);
    }

    fn.AppendDir("extension");
  }
  else
  {
    for (const auto& it : ar)
    {
      if (it == "build" || it == "Release" || 
          it == "Debug" || it == "Coverage")
      {
        fn.AppendDir("extension");
        break;      
      }
    
      fn.AppendDir(it);
    
      if (it == "extension")
      {
        break;
      }
    }
  }
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
  
wxFileName wxExTestApp::m_TestFileName;

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui");
  SetWorkingDirectory();
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
  
const std::string wxExTestApp::SetWorkingDirectory()
{
  const wxString old = wxGetCwd();

  m_TestFileName = wxFileName(old, "");
  
  if (m_TestFileName.GetDirs().Index("wxExtension") == wxNOT_FOUND)
  {
    if (m_TestFileName.GetDirs().Index("extension") == wxNOT_FOUND)
    {
      m_TestFileName.RemoveLastDir();
      m_TestFileName.AppendDir("extension");
    }
    else
    {
      SetFindExtension(m_TestFileName);
    }
  }
  else
  {
    SetFindExtension(m_TestFileName);
  }
  
  if (m_TestFileName.GetDirs().Index("test") == wxNOT_FOUND)
  {
    m_TestFileName.AppendDir("test");
    m_TestFileName.AppendDir("data");
  }
  
  if (!wxSetWorkingDirectory(m_TestFileName.GetFullPath()))
  {
    fprintf(stderr, "%s\n", (const char *)m_TestFileName.GetFullPath().c_str());
    exit(1);
  }
  
  return old.ToStdString();
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
