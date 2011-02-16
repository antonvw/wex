/******************************************************************************\
* File:          main.cpp
* Purpose:       main for wxExtension report cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: main.cpp 589 2009-04-09 13:43:53Z antonvw $
* Created:       zo 08 mrt 2009 16:16:29 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include <wx/extension/report/frame.h>
#include "test.h"

wxIMPLEMENT_APP(wxExReportTestApp);

bool wxExReportTestApp::OnInit()
{
  SetAppName("wxex-test-rep");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExFrameWithHistory *frame = new 
    wxExFrameWithHistory(NULL, wxID_ANY, wxTheApp->GetAppDisplayName());
    
  frame->Show(true);

  CppUnit::TextUi::TestRunner runner;

  wxExReportTestSuite* suite = new wxExReportTestSuite;

  runner.addTest(suite);
  runner.run();

  // Return false, so test ends here.
  return false;
}
