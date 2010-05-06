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
#include <wx/extension/extension.h>

/// CppUnit test suite.
class wxExTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  wxExTestSuite();
};

/// Derive your application from wxExApp.
class wxExTestApp: public wxExApp
{
public:
  /// Constructor.
  wxExTestApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
};

/// CppUnit app test fixture.
/// These classes require either an wxExApp object, or wx to be initialized.
class wxExAppTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExAppTestFixture() : TestFixture() {
    m_Grid = NULL;
    m_ListView = NULL;
    m_Notebook = NULL;
    m_STC = NULL;
    m_STCShell = NULL;
    m_VCS = NULL;
    };

  /// Destructor.
 ~wxExAppTestFixture() {
    delete m_Grid;
    delete m_ListView;
    delete m_Notebook;
    delete m_STC;
    delete m_STCShell;
    delete m_VCS;
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
  wxExGrid* m_Grid;   ///< testing wxExGrid
  wxExListView* m_ListView; ///< testing wxExListView
  wxExNotebook* m_Notebook; ///< testing wxExNotebook
  wxExSTCShell* m_STCShell; ///< testing wxExSTCShell
  wxExSTC* m_STC;     ///< testing wxExSTC
  wxExVCS* m_VCS;     ///< testing wxExVCS
};
#endif
