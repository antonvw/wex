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

#ifndef _EXTESTCASE_H
#define _EXTESTCASE_H

#include <iostream>
#include <string>
#include <TestCase.h>
#include <TestSuite.h>
#include <TestCaller.h>
#include <TestRunner.h>

/// CppUnit test case.
class exTestCase : public CppUnit::TestCase
{
public:
  /// Constructor.
  exTestCase(std::string name) : TestCase(name) {}

  /// Test the constructor of various extension classes.
  void testConstructor();

  /// Test various methods of various extension classes.
  void testMethods();

  /// Method to create a suite of tests.
  static CppUnit::Test* suite();
};
#endif
