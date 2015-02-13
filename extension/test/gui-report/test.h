////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _FTTESTUNIT_H
#define _FTTESTUNIT_H

#include <cppunit/extensions/HelperMacros.h>
#include <wx/extension/app.h>
#include <wx/extension/report/frame.h>
#include "../test.h"

class FrameWithHistory : public wxExFrameWithHistory
{
public:
  FrameWithHistory(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    size_t maxFiles = 9,
    size_t maxProjects = 0,
    int style = wxDEFAULT_FRAME_STYLE);

  virtual wxExListViewFileName* Activate(
    wxExListViewFileName::wxExListType list_type, 
    const wxExLexer* lexer);
private:
  wxExListViewFileName* m_Report;
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
  virtual int OnRun();
};

/// CppUnit app test fixture.
/// These classes require either an wxExApp object, or wx to be initialized.
class wxExGuiReportTestFixture : public wxExTestFixture
{
  CPPUNIT_TEST_SUITE( wxExGuiReportTestFixture );
  
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
  wxExGuiReportTestFixture();
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
  FrameWithHistory* m_Frame;
};
#endif
