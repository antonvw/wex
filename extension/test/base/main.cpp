////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <wx/utils.h>

#include "test.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestFixture );

int main (int argc, char* argv[])
{
  SetWorkingDirectory();
  SetEnvironment(wxGetHomeDir() + "/.wxex-test-base");
  
  CppUnit::TestResult result;
  CppUnit::TestResultCollector collector;
  result.addListener( &collector );        
  CppUnit::BriefTestProgressListener progressListener;
  result.addListener( &progressListener );    
  CppUnit::TestRunner runner;
  runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

  runner.run(result);
}
