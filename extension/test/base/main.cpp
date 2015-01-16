////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestRunner.h>
#include <wx/filename.h>
#include <wx/log.h>

#include "test.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestFixture );

int main (int argc, char* argv[])
{
  CppUnit::TextUi::TestRunner runner;
  
  wxLog::SetActiveTarget(new wxLogStderr());
  
  wxString dir("~/.wxex-test-base"); // wxStandardPaths::Get().GetUserDataDir())
  
  if (!wxDirExists(dir))
  {
    system("mkdir "+ dir);
    system("cp ../extension/data/*.xml " + dir);
  }
    
  // Get the top level suite from the registry
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  runner.addTest(suite);

  return runner.run("", false);
}
