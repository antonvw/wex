////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
  
  void testConfig();
  void testDir();
  void testFile();
  void testFileName();
  void testFileStatistics();
  void testStat();
  void testStatistics();
  void testTextFile();
  void testTiming();
  void testTimingAttrib();
  void testTool();
};

#endif
