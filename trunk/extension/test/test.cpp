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

#include <wx/extension/extension.h>
#include "test.h"

using CppUnit::Test;

void exTestCase::testConstructor() 
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
    "testConstructor", 
    &exTestCase::testConstructor);
    
  CPPUNIT_NS::TestCaller* test2 = new CPPUNIT_NS::TestCaller(
    "testMethods", 
    &exTestCase::testMethods);
    
  // Add the tests.
  CPPUNIT_NS::TestSuite* testSuite = new CPPUNIT_NS::TestSuite("wxExtensionTestCase");
  testSuite->addTest(test1);    
  testSuite->addTest(test2);
       
  return testSuite;
}

int main (int argc, char* argv[]) 
{
  if (argc != 2)
  {
    std::cout << "usage: tester name_of_class_being_test" << std::endl;
    exit(1);
  }

  CPPUNIT_NS::TestRunner runner;
  runner.addTest(exTestCase::suite());
  runner.run("", false, true);
  
  return 0;
}
