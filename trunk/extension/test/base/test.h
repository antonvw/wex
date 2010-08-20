/******************************************************************************\
* File:          test.h
* Purpose:       Declaration of classes for wxExtension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXTESTUNIT_H
#define _EXTESTUNIT_H

#include <TestFixture.h>
#include <TestSuite.h>

/// CppUnit test suite.
class wxExTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  wxExTestSuite();
};

/// CppUnit base test fixture.
class wxExTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExTestFixture() : TestFixture() {};

  /// Destructor.
 ~wxExTestFixture() {};

  /// Set up context before running a test.
  virtual void setUp() {};

  /// Clean up after the test run.
  virtual void tearDown() {};
  
  void testDir();
  void testFile();
  void testFileName();
  void testFileStatistics();
  void testRCS();
  void testStat();
  void testStatistics();
  void testTextFile();
  void testTiming();
  void testTimingAttrib();
  void testTool();
};

#endif
