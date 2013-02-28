////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013
////////////////////////////////////////////////////////////////////////////////

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include "test.h"

int main (int argc, char* argv[])
{
  CppUnit::TextUi::TestRunner runner;

  wxLog::SetActiveTarget(new wxLogStderr());
    
  wxExTestSuite* suite = new wxExTestSuite;

  runner.addTest(suite);
  bool success = runner.run("", false);
  
  return !success;
}
