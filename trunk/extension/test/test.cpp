/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <ui/text/TestRunner.h>
#include <TestSuite.h>
#include <TestCaller.h>
#include <wx/extension/extension.h>
#include "test.h"

using CppUnit::Test;

void exTestCase::testConstructors() 
{
  exFile file("test.h");
  
  assert(file.GetStat().IsOk());
}

void exTestCase::testMethods() 
{
}

Test* exTestCase::suite()
{
  // Construct the tests.
  CPPUNIT_NS::TestCaller* test1 = new CPPUNIT_NS::TestCaller(
    "testConstructors", 
    &exTestCase::testConstructors);
    
  CPPUNIT_NS::TestCaller* test2 = new CPPUNIT_NS::TestCaller(
    "testMethods", 
    &exTestCase::testMethods);
    
  // Add the tests.
  CPPUNIT_NS::TestSuite* testSuite = new CPPUNIT_NS::TestSuite("wxextension test suite");
  testSuite->addTest(test1);    
  testSuite->addTest(test2);
       
  return testSuite;
}

int main (int argc, char* argv[]) 
{
  CppUnit::TextUi::TestRunner runner;
  
  runner.addTest(exTestCase::suite());
  runner.run();
  
  return 0;
}
