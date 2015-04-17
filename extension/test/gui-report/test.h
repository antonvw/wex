////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <wx/extension/report/frame.h>
#include "../test.h"

/// CppUnit app test fixture.
/// These classes require either an wxExApp object, or wx to be initialized.
class fixture : public wxExTestFixture
{
  CPPUNIT_TEST_SUITE( fixture );
  
  CPPUNIT_TEST( testDirCtrl );
  CPPUNIT_TEST( testDirTool );
  CPPUNIT_TEST( testDirWithListView );
  CPPUNIT_TEST( testFrameWithHistory );
  CPPUNIT_TEST( testListViewFile );
  CPPUNIT_TEST( testListViewWithFrame );
  CPPUNIT_TEST( testTextFileWithListView );
  CPPUNIT_TEST( testUtil );
  CPPUNIT_TEST( test );
  
  CPPUNIT_TEST_SUITE_END();

public:
  fixture();
  void testDirCtrl();
  void testDirTool();
  void testDirWithListView();
  void testFrameWithHistory();
  void testListViewFile();
  void testListViewWithFrame();
  void testTextFileWithListView();
  void testUtil();
  
  // integration test
  void test();
private:
  const wxString m_Project;
  static wxExFrameWithHistory* m_Frame;
};
