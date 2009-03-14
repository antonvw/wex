/******************************************************************************\
* File:          test.h
* Purpose:       Declaration of classes for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: test.h 410 2009-02-28 14:48:59Z antonvw $
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
#include <wx/extension/extension.h>
#include <wx/extension/dir.h>
#include <wx/extension/grid.h>
#include <wx/extension/listview.h>
#include <wx/extension/stc.h>
#include <wx/extension/svn.h>
#include <wx/extension/shell.h>
#include <wx/extension/textfile.h>

/// CppUnit test suite.
class exTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  exTestSuite();
};

/// Derive your application from exApp.
class exTestApp: public exApp
{
public:
  /// Constructor.
  exTestApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
};

/// CppUnit app test fixture.
/// These classes require either an exApp object, or wx to be initialized.
class exAppTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  exAppTestFixture() : TestFixture() {
    m_Dir = NULL;
    m_Grid = NULL;
    m_ListView = NULL;
    m_STC = NULL;
    m_STCShell = NULL;
    m_SVN = NULL;
    };

  /// Destructor.
 ~exAppTestFixture() {
    delete m_Dir;
    delete m_Grid;
    delete m_ListView;
    delete m_STC;
    delete m_STCShell;
    delete m_SVN;
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
  exDir* m_Dir;     ///< testing exDir
  exGrid* m_Grid;   ///< testing exDir
  exListView* m_ListView; ///< testing exDir
  exSTCShell* m_STCShell; ///< testing exSTC
  exSTC* m_STC;     ///< testing exSTC
  exSVN* m_SVN;     ///< testing exSVN
};
#endif
