////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/log.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
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
  if (wxTheApp != NULL)
  {
    const long verbose(wxConfigBase::Get()->ReadLong("verbose", 0));
    
    switch (verbose)
    {
      case 1: wxLog::SetActiveTarget(new wxLogStderr()); break;
      case 2: new wxLogNull(); break;
    }
    
    wxConfigBase::Get()->Write(_("vi mode"), true);
    
    // Create the global lexers object, 
    // it should be present in ~/.wxex-test-gui
    // (depending on platform, configuration).
    wxExLexers::Get();
  }
    
  if (!wxDirExists(dir))
  {
    (void)system("mkdir " + dir);
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
  
  SetEnvironment(
#ifdef wxExUSE_PORTABLE
    wxPathOnly(wxStandardPaths::Get().GetExecutablePath()));
#else
    wxStandardPaths::Get().GetUserDataDir());
#endif
  
  if (!wxExApp::OnInit())
  {
    return false;
  }
  
  return true;
}
