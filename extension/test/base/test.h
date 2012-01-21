////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXBASETESTUNIT_H
#define _EXBASETESTUNIT_H

#include <TestFixture.h>
#include <TestSuite.h>
#include <wx/extension/extension.h>

/// CppUnit test suite.
class wxExTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  wxExTestSuite();
};

/// CppUnit base test fixture.
class TestFixture : public wxExTestFixture
{
public:
  /// Default constructor.
  TestFixture() : wxExTestFixture() {;};

  /// Destructor.
 ~TestFixture() {};
 
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
