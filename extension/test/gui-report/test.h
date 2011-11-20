////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _FTTESTUNIT_H
#define _FTTESTUNIT_H

#include <TestFixture.h>
#include <TestSuite.h>
#include <wx/extension/app.h>

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
  wxExReportAppTestFixture() : TestFixture() {};

  /// Destructor.
 ~wxExReportAppTestFixture() {};

  /// From TestFixture.
  /// Set up context before running a test.
  virtual void setUp() {};

  /// Clean up after the test run.
  virtual void tearDown() {};

  void testConfig();
  void testDirWithListView();
  void testFrameWithHistory();
  void testListItem();
  void testListViewFile();
  void testProcess();
  void testSTCWithFrame();
};
#endif

