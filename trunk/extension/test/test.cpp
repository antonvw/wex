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
using CppUnit::TestSuite;
using CppUnit::TestFactoryRegistry;
using CppUnit::TextUi::TestRunner;
using CppUnit::CompilerOutputter;

// method to test the constructor
void StudentTestCase::testConstructor() 
{
  exFile file("test.h");
  
  assert(file.GetStat().IsOk());
}

// method to test the assigning and retrieval of grades
void StudentTestCase::testAssignAndRetrieveGrades() 
{
}

// method to create a suite of tests - Note 7
Test* StudentTestCase::suite()
{
  TestSuite* testSuite = new TestSuite("wxExtensionTestCase");
  
  // add the tests
  testSuite->addTest(new TestCaller(
    "testConstructor", 
    &StudentTestCase::testConstructor));
    
  testSuite->addTest(new TestCaller(
    "testAssignAndRetrieveGrades", 
    &StudentTestCase::testAssignAndRetrieveGrades));
       
  return testSuite;
}

// the main method - Note 8
int main (int argc, char* argv[]) 
{
  if (argc != 2)
  {
    std::cout << "usage: tester name_of_class_being_test" << std::endl;
    exit(1);
  }

  TestRunner runner;
  runner.addTest(argv[1], StudentTestCase::suite());
  runner.run(argc, argv);
  
  return 0;
}
