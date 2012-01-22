////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include <wx/extension/report/frame.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui-report");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExFrameWithHistory *frame = new 
    wxExFrameWithHistory(NULL, wxID_ANY, wxTheApp->GetAppDisplayName());
    
  frame->Show(true);

  wxLog::SetActiveTarget(new wxLogStderr());
    
  CppUnit::TextUi::TestRunner runner;

  wxExTestSuite* suite = new wxExTestSuite;

  runner.addTest(suite);
  runner.run();

  wxExTestFixture::PrintReport();
  
  // Return false, so test ends here.
  return false;
}
