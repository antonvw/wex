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

#ifndef _EXTESTFIXTURE_H
#define _EXTESTFIXTURE_H

#include <iostream>
#include <string>
#include <TestFixture.h>
#include <wx/extension/extension.h>

/// CppUnit test case.
class exTestFixture : public CppUnit::TestFixture
{
public:
  /// Constructor.
  exTestFixture() : TestFixture() {}

  /// From TestFixture.
  /// Set up context before running a test. 
  virtual void setUp();
  
  /// Clean up after the test run.
  virtual void tearDown();

  /// Test the constructors of various extension classes.
  void testConstructors();

  /// Test methods of various extension classes.
  void testMethods();

  /// Create a suite of tests.
  static CppUnit::Test* suite();
private:
  exFile* m_File; ///< testing exFile
  exFileName* m_FileName; ///< testing exFileName
  exStat* m_Stat; ///< testing exStat
};
#endif
