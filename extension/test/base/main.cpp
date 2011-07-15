////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011
////////////////////////////////////////////////////////////////////////////////

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include <wx/log.h>
#include "test.h"

int main (int argc, char* argv[])
{
  CppUnit::TextUi::TestRunner runner;

  wxLog::SetActiveTarget(new wxLogStderr());
    
  wxExTestSuite* suite = new wxExTestSuite;

  runner.addTest(suite);
  const bool wasSucessful = runner.run();
  
  // Return error code 1 if the one of test failed.
  return !wasSucessful;
}

