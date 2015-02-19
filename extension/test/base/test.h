////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "../test.h"

/// CppUnit base test fixture.
class TestFixture : public wxExTestFixture
{
  CPPUNIT_TEST_SUITE( TestFixture );
  
  CPPUNIT_TEST( testDir);
  CPPUNIT_TEST( testFile);
  CPPUNIT_TEST( testFileTiming);
  CPPUNIT_TEST( testFileName);
  CPPUNIT_TEST( testFileNameTiming);
  CPPUNIT_TEST( testInterruptable);
  CPPUNIT_TEST( testStat);
  CPPUNIT_TEST( testStatistics);
  CPPUNIT_TEST( testTool );
  
  CPPUNIT_TEST_SUITE_END();

public:
  void testDir();
  void testFile();
  void testFileTiming();
  void testFileName();
  void testFileNameTiming();
  void testInterruptable();
  void testStat();
  void testStatistics();
  void testTool();
};
