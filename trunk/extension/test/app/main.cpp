/******************************************************************************\
* File:          main.cpp
* Purpose:       main for wxExtension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-app");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExManagedFrame *frame = new wxExManagedFrame(NULL, 
    wxID_ANY, wxTheApp->GetAppDisplayName());
    
  frame->Show(true);

  CppUnit::TextUi::TestRunner runner;

  wxExAppTestSuite* suite = new wxExAppTestSuite;

  runner.addTest(suite);
  runner.run();
  
  // Return false, so test ends here.
  return false;
}
