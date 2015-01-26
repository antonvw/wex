////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <wx/log.h>
#include "test.h"

//#define SHOW_REPORT

void SetEnvironment(const wxString& dir)
{
  wxLog::SetActiveTarget(new wxLogStderr());
  
  if (!wxDirExists(dir))
  {
    system("mkdir " + dir);
  }
  
  system("cp ../../data/lexers.xml " + dir);
  system("cp ../../data/macros.xml " + dir);
  system("cp ../../data/vcs.xml " + dir);
}
    
void SetFindExtension(wxFileName& fn)
{
  const wxArrayString ar(fn.GetDirs());
  
  fn.Assign("/", "");

  for (int i = 0; i < ar.GetCount(); i++)
  {
    if (ar[i] == "build" || ar[i]== "Release" || 
       ar[i] == "Debug" || ar[i] == "Coverage")
    {
      fn.AppendDir("extension");
      break;      
    }
    
    fn.AppendDir(ar[i]);
    
    if (ar[i] == "extension")
    {
      break;
    }
  }
    
  fprintf(stderr, "EXT: %s\n", (const char *)fn.GetFullPath().c_str());
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
  
  fprintf(stderr, "WD: %s\n", (const char *)fn.GetFullPath().c_str());
  
  return old;
}

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
