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
#include <wx/extension/extension.h>

/// CppUnit test suite.
class wxExAppTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  wxExAppTestSuite();
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

/// CppUnit gui test fixture.
/// These classes require either an wxExApp object, or wx to be initialized.
class wxExGuiTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExGuiTestFixture() : TestFixture() {}; 
  
  /// Destructor.
 ~wxExGuiTestFixture() {};

  /// From TestFixture.
  /// Set up context before running a test.
  virtual void setUp();

  /// Clean up after the test run.
  virtual void tearDown() {};

  void testConfigDialog();
  void testConfigItem();
  void testDialog();
  void testFrame();
  void testFrd();
  void testGrid();
  void testHeader();
  void testHexMode();
  void testIndicator();
  void testLexer();
  void testLexers();
  void testListItem();
  void testListView();
  void testManagedFrame();
  void testMarker();
  void testMenu();
  void testNotebook();
  void testOTL();
  void testPrinting();
  void testProcess();
  void testProperty();
  void testShell();
  void testStatusBar();
  void testSTC();
  void testSTCFile();
  void testStyle();
  void testUtil();
  void testVCS();
  void testVCSCommand();
  void testVCSEntry();
  void testVi();
};
#endif
