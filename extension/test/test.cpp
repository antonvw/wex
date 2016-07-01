////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016
////////////////////////////////////////////////////////////////////////////////

#define CATCH_CONFIG_RUNNER

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <functional>
#include <wx/config.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/timer.h>
#include "wx/uiaction.h"
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/util.h>
#include "catch.hpp"
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

const wxString GetTestDir()
{
  return wxString(".") + wxFileName::GetPathSeparator();
}
  
const wxExFileName GetTestFile()
{
  return GetTestDir() + "test.h";
}
  
const wxString BuildArg(const wxString& file)
{
  return 
    wxString(" ..") + wxFileName::GetPathSeparator() + 
    ".." + wxFileName::GetPathSeparator() + 
    "data" + wxFileName::GetPathSeparator() + 
    file + " ";
}

void SetEnvironment(const wxString& dir)
{
  if (!wxDirExists(dir))
  {
    (void)system("mkdir -p " + dir);
  }

#ifdef __UNIX__
  const wxString cp("cp");
#else
  const wxString cp("COPY");
#endif
  
  (void)system(cp + BuildArg("cht.txt") + dir);
  (void)system(cp + BuildArg("lexers.xml") + dir);
  (void)system(cp + BuildArg("macros.xml") + dir);
  (void)system(cp + BuildArg("vcs.xml") + dir);

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
    
const wxString SetWorkingDirectory()
{
  const wxString old = wxGetCwd();

  wxFileName fn(old, "");
  
  if (fn.GetDirs().Index("wxExtension") == wxNOT_FOUND)
  {
    if (fn.GetDirs().Index("extension") == wxNOT_FOUND)
    {
      fn.RemoveLastDir();
      fn.AppendDir("extension");
    }
    else
    {
      SetFindExtension(fn);
    }
  }
  else
  {
    SetFindExtension(fn);
  }
  
  if (fn.GetDirs().Index("test") == wxNOT_FOUND)
  {
    fn.AppendDir("test");
    fn.AppendDir("data");
  }
  
  if (!wxSetWorkingDirectory(fn.GetFullPath()))
  {
    fprintf(stderr, "%s\n", (const char *)fn.GetFullPath().c_str());
    exit(1);
  }
  
  return old;
}

bool wxExUIAction(wxWindow* win, const wxString& action, const wxString& par)
{
  wxUIActionSimulator sim;

  if (action.StartsWith("button"))
  {
    sim.MouseMove(win->GetScreenPosition() + wxPoint(100, 100));
    
    if (par.Contains("left") || par.Contains("right"))
    {
      sim.MouseClick(par.Contains("right") ? wxMOUSE_BTN_RIGHT: wxMOUSE_BTN_LEFT);
    }
  }
  else if (action.StartsWith("key"))
  {
  }
  else if (action.StartsWith("toolbar"))
  {
    sim.MouseMove(win->GetScreenPosition() + wxPoint(5, 5));
    sim.MouseClick(wxMOUSE_BTN_LEFT);
  }
  else
  {
    return false;
  }
  
  wxTimer* timer = new wxTimer(wxTheApp);
  timer->StartOnce(1000);
  wxTheApp->Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    wxUIActionSimulator sim;
    sim.Char(WXK_RETURN);});

  return true;
}
  
int wxExTestApp::OnExit()
{
  // Remove files.
  (void)remove("test-ex.txt");
  (void)remove("test-file.txt");
  (void)remove("test.hex");
  (void)remove("test-xxx");

  return wxExApp::OnExit();
}
  
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
    const int fails = m_Session->run();
    const long auto_exit(wxConfigBase::Get()->ReadLong("auto-exit", 1));
    wxExUIAction(GetTopWindow(), "key", "char");
    if (auto_exit && (argc < 2 || fails == 0))
    {
      OnExit();
      exit(fails > 0 ? EXIT_FAILURE: EXIT_SUCCESS);
    }});

  return wxExApp::OnRun();
}

void wxExTestApp::SetSession(Catch::Session* session)
{
  m_Session = session;
}
  
int wxExTestMain(int argc, char* argv[], wxExTestApp* app, bool use_eventloop)
{
  Catch::Session session; // There must be exactly once instance

  int returnCode = session.applyCommandLine(argc, (const char **)argv);
  
  if (returnCode != 0 || session.configData().showHelp)
    return returnCode;

  wxApp::SetInstance(app);
  wxEntryStart(argc, argv);
  app->OnInit();
  
  if (!use_eventloop)
  {
    const int fails = session.run();
    app->ProcessPendingEvents();
    app->OnExit();
    app->ExitMainLoop();
    return fails > 0 ? EXIT_FAILURE: EXIT_SUCCESS;
  }
  else
  {
    app->SetSession(&session);
    return app->OnRun();
  }
}
