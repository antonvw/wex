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
  // Add the tests.
  CppUnit::TestSuite* testSuite = new CppUnit::TestSuite("wxextension test suite");
  testSuite->addTest(new CppUnit::TestCaller<exTestCase>(
    "testConstructors", 
    &exTestCase::testConstructors));    
  testSuite->addTest(new CppUnit::TestCaller<exTestCase>(
    "testMethods", 
    &exTestCase::testMethods));
       
  return testSuite;
}

int main (int argc, char* argv[]) 
{
  CppUnit::TextUi::TestRunner runner;
  
  runner.addTest(exTestCase::suite());
  runner.run();
  
  return 0;
}
