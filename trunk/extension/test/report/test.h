/******************************************************************************\
* File:          test.h
* Purpose:       Declaration of classes for wxExReport cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: test.h 589 2009-04-09 13:43:53Z antonvw $
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
#include <wx/extension/report/report.h>

/// CppUnit test suite.
class wxExReportTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  wxExReportTestSuite();
};

/// Derive your application from wxExApp.
class wxExReportTestApp: public wxExApp
{
public:
  /// Constructor.
  wxExReportTestApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
};

/// CppUnit app test fixture.
/// These classes require either an wxExApp object, or wx to be initialized.
class wxExReportAppTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExReportAppTestFixture() : TestFixture() {
    m_Dir = NULL;
    m_Frame = NULL;
    m_ListView = NULL;
    m_Process = NULL;
    m_STC = NULL;
    };

  /// Destructor.
 ~wxExReportAppTestFixture() {
    delete m_Dir;
    delete m_Frame;
    delete m_ListView;
    delete m_Process;
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
  wxExDirWithReport* m_Dir;     ///< testing wxExDirWithReport
  wxExFrameWithHistory* m_Frame; ///< testing wxExFrameWithHistory
  wxExListViewFile* m_ListView; ///< testing wxExListViewFile
  wxExProcessWithListView* m_Process; ///< testing wxExProcessWithListView
  wxExSTCWithFrame* m_STC;      ///< testing wxExSTCWithFrame
};
#endif

