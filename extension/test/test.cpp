////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestRunner.h>
#include <wx/cmdline.h> // for wxCmdLineParser
#include <wx/stdpaths.h>
#include <wx/log.h>
#include "test.h"

#define LOGGING ON

void SetEnvironment(const wxString& dir)
{
  wxLog::SetActiveTarget(new wxLogStderr());
  
  if (!wxDirExists(dir))
  {
    (void)system("mkdir " + dir);
  }
  
  (void)system("cp ../../data/lexers.xml " + dir);
  (void)system("cp ../../data/macros.xml " + dir);
  (void)system("cp ../../data/vcs.xml " + dir);
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
  // Is not invoked...
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

void wxExTestApp::OnInitCmdLine(wxCmdLineParser& parser)
{
  wxExApp::OnInitCmdLine(parser);

  parser.AddParam(
    "test",
    wxCMD_LINE_VAL_STRING,
    wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
}

int wxExTestApp::OnRun()
{
  std::string name = "";
  
  if (argc > 1)
  {
    name = argv[1];
  }
      
  try
  {
    CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
    
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);
    
    const bool success = runner.run(name, false);
    
    // Remove files.
    (void)remove("test-ex.txt");
    (void)remove("test.hex");
  
    exit(!success);
  }
  catch (std::invalid_argument e)
  {
    printf("invalid test: %s\n", name.c_str());
    return 0;
  }
  
  return 0;
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
