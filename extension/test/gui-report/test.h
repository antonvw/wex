////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _FTTESTUNIT_H
#define _FTTESTUNIT_H

#include <wx/extension/extension.h>
#include "../test.h"

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
  wxExTestApp() : m_Success(false) {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
  virtual int OnRun();
  
  bool m_Success;
};

/// CppUnit app test fixture.
/// These classes require either an wxExApp object, or wx to be initialized.
class wxExGuiReportTestFixture : public wxExTestFixture
{
public:
  /// Default constructor.
  wxExGuiReportTestFixture() : wxExTestFixture() {;};

  /// Destructor.
 ~wxExGuiReportTestFixture() {};

  /// From TestFixture.
  /// Set up context before running a test.
  virtual void setUp() {wxExTestFixture::setUp();};

  /// Clean up after the test run.
  virtual void tearDown() {wxExTestFixture::tearDown();};

  void testDirCtrl();
  void testDirTool();
  void testDirWithListView();
  void testFrameWithHistory();
  void testListViewFile();
  void testListViewWithFrame();
  void testSTCWithFrame();
  void testTextFileWithListView();
  void testUtil();
  
  // integration test
  void test();
};
#endif

