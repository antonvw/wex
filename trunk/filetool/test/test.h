/******************************************************************************\
* File:          test.h
* Purpose:       Declaration of classes for wxfiletool cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _FTTESTUNIT_H
#define _FTTESTUNIT_H

#include <TestFixture.h>
#include <TestSuite.h>
#include <wx/filetool/filetool.h>

/// CppUnit test suite.
class ftTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  ftTestSuite();
};

/// Derive your application from exApp.
class ftTestApp: public exApp
{
public:
  /// Constructor.
  ftTestApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
};

/// CppUnit app test fixture.
/// These classes require either an exApp object, or wx to be initialized.
class ftAppTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  ftAppTestFixture() : TestFixture() {
    m_STC = NULL;
    };

  /// Destructor.
 ~ftAppTestFixture() {
    delete m_STC;
    };

  /// From TestFixture.
  /// Set up context before running a test.
  virtual void setUp();

  /// Clean up after the test run.
  virtual void tearDown();

  /// Test the constructors of various extension classes.
  void testConstructors();

  /// Test methods of various extension classes requiring app.
  void testMethods();
private:
  ftSTC* m_STC;     ///< testing ftSTC
};
#endif

