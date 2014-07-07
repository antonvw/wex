////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014
////////////////////////////////////////////////////////////////////////////////

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include <wx/msgdlg.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExManagedFrame* frame = new 
    wxExManagedFrame(NULL, wxID_ANY, wxTheApp->GetAppDisplayName());
    
  frame->Show(true);
  wxLog::SetActiveTarget(new wxLogStderr());
  
  wxLogStatus(GetCatalogDir());
  wxLogStatus(GetLocale().GetLocale());
  
  return true;
}

int wxExTestApp::OnRun()
{
  CppUnit::TextUi::TestRunner runner;

  wxExAppTestSuite* suite = new wxExAppTestSuite;
  runner.addTest(suite);
  
  return !runner.run("", false);
}
