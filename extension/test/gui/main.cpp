////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012
////////////////////////////////////////////////////////////////////////////////

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExManagedFrame *frame = new wxExManagedFrame(NULL, 
    wxID_ANY, wxTheApp->GetAppDisplayName());
    
  frame->Show(true);

  wxLog::SetActiveTarget(new wxLogStderr());
    
  CppUnit::TextUi::TestRunner runner;

  wxExAppTestSuite* suite = new wxExAppTestSuite;

  runner.addTest(suite);
  runner.run();
  
  wxExTestFixture::PrintReport();
  
  // Return false, so test ends here.
  return false;
}
