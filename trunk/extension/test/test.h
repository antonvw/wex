/******************************************************************************\
* File:          test.h
* Purpose:       Declaration of classes for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef TestStudent_h
#define TestStudent_h

#include <iostream>
#include <string>

#include <TestCase.h>
#include <TestSuite.h>
#include <TestCaller.h>
#include <TestRunner.h>

class StudentTestCase : public CppUnit::TestCase
{
public:
  // constructor - Note 3
  StudentTestCase(std::string name) : TestCase(name) {}

  // method to test the constructor
  void testConstructor();

  // method to test the assigning and retrieval of grades
  void testAssignAndRetrieveGrades();

  // method to create a suite of tests
  static CppUnit::Test* suite();
};
#endif
