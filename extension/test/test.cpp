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

const wxString BuildArg(const wxString& file)
{
  return 
    wxString(" ..") + wxFileName::GetPathSeparator() + 
    ".." + wxFileName::GetPathSeparator() + 
    "data" + wxFileName::GetPathSeparator() + 
    file + " ";
}

const std::string GetTestDir()
{
  return wxExTestApp::GetTestFileName().GetFullPath().ToStdString() + 
    (char)wxFileName::GetPathSeparator();
}
  
const wxExFileName GetTestFile()
{
  return GetTestDir() + "test.h";
}
  
void SetEnvironment(const std::string& dir)
{
  if (!wxDirExists(dir))
  {
    (void)system("mkdir -p " + wxString(dir));
  }

#ifdef __UNIX__
  const wxString cp("cp");
#else
  const wxString cp("COPY");
#endif
  
  (void)system(cp + BuildArg("cht.txt") + dir);
  (void)system(cp + BuildArg("lexers.xml") + dir);
  (void)system(cp + BuildArg("macros.xml") + dir);
  (void)system(cp + BuildArg("menus.xml") + dir);

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
    
bool wxExUIAction(wxWindow* win, const wxString& action, const wxString& par)
{
#if wxCHECK_VERSION(3,1,0)
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
      const int fails = m_Session->run();
      wxExUIAction(GetTopWindow(), "key", "char");
      const long auto_exit(wxConfigBase::Get()->ReadLong("auto-exit", 1));
      if (auto_exit)
      {
        exit(fails > 0 ? EXIT_FAILURE: EXIT_SUCCESS);
      }
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << "\n";
      exit(EXIT_FAILURE);
    }});

  return wxExApp::OnRun();
}

void wxExTestApp::SetSession(Catch::Session* session)
{
  m_Session = session;
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
  Catch::Session session; // There must be exactly one instance

  int returnCode = session.applyCommandLine(argc, (const char **)argv);
  
  if (returnCode != 0 || session.configData().showHelp)
    return returnCode;

  wxApp::SetInstance(app);
  wxEntryStart(argc, argv);
  app->OnInit();
  
  if (!use_eventloop)
  {
    try
    {
      const int fails = session.run();
      app->ProcessPendingEvents();
      app->ExitMainLoop();
      return fails > 0 ? EXIT_FAILURE: EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << "\n";
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    app->SetSession(&session);
    return app->OnRun();
  }
}
