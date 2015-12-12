////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <wx/cmdline.h> // for wxCmdLineParser
#include <wx/stdpaths.h>
#include <wx/log.h>
#include <wx/extension/managedframe.h>
#include "test.h"

#define LOGGING ON

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

void SetEnvironment(const wxString& dir)
{
  wxLog::SetActiveTarget(new wxLogStderr());
  
  if (!wxDirExists(dir))
  {
    (void)system("mkdir " + dir);
  }
  
  (void)system("cp ../../data/cht.txt " + dir);
  (void)system("cp ../../data/lexers.xml " + dir);
  (void)system("cp ../../data/macros.xml " + dir);
  (void)system("cp ../../data/vcs.xml " + dir);
  
#if wxExUSE_OTL
  (void)system("cp .odbc.ini " + wxGetHomeDir());
#endif
}
    
void SetFindExtension(wxFileName& fn)
{
  const wxArrayString ar(fn.GetDirs());
  const int index = ar.Index("wxExtension");
  
  fn.Assign("/", "");
  
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

#ifdef LOGGING    
  fprintf(stderr, "EXT: %s\n", (const char *)fn.GetFullPath().c_str());
#endif  
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
  
#ifdef LOGGING    
  fprintf(stderr, "WD: %s\n", (const char *)fn.GetFullPath().c_str());
#endif  
  
  return old;
}

int wxExTestApp::OnExit()
{
  // Remove files.
  (void)remove("test-ex.txt");
  (void)remove("test.hex");

  return wxExApp::OnExit();
}
  
bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui");
  SetWorkingDirectory();
  SetEnvironment(wxStandardPaths::Get().GetUserDataDir());
  
  if (!wxExApp::OnInit())
  {
    return false;
  }
  
  wxLogStatus(GetCatalogDir());
  wxLogStatus(GetLocale().GetLocale());
  
  return true;
}

int wxExTestApp::OnRun()
{
  std::vector<wxString> names;
  
  if (argc > 1)
  {
    for (int i = 1; i < argc; i++)
    {
      names.push_back(argv[i]);
    }
  }
      
  try
  {
    CppUnit::TestResult result;
    CppUnit::TestResultCollector collector;
    result.addListener( &collector );        
    CppUnit::BriefTestProgressListener progressListener;
    result.addListener( &progressListener );    
    
    CppUnit::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    
    if (names.empty())
    {
      runner.run(result);
    }
    else
    {
      for (auto it : names)
      {
        runner.run(result, it.ToStdString());
      }
    }
    
    // Print test in a compiler compatible format.
    CppUnit::CompilerOutputter outputter(&collector, std::cerr);
    outputter.write();                      
    
    if (argc <= 1)
    {
      OnExit();
      exit(collector.testFailures() > 0 ? EXIT_FAILURE: EXIT_SUCCESS);
    }
    else
    {
      wxExApp::OnRun();
    }
  }
  catch (std::invalid_argument& e)
  {
    printf("invalid test: %s\n", e.what());
    return 0;
  }
}

//#define SHOW_REPORT

wxExTestFixture::wxExTestFixture() 
  : TestFixture() 
  , m_TestDir("./")
  , m_TestFile(m_TestDir + "test.h")
{
}

void wxExTestFixture::tearDown() 
{
#ifdef SHOW_REPORT
  if (!m_Report.empty()) 
    std::cout << m_Report;
#endif  
}
      
void wxExTestFixture::Report(const std::string& text) 
{
  m_Report.append(text);
  m_Report.append("\n");
}
