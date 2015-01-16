////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

void wxExGuiReportTestFixture::testFrameWithHistory()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  
  CPPUNIT_ASSERT(!frame->OpenFile(GetTestFile())); // as we have no focused stc
  CPPUNIT_ASSERT(!frame->GetRecentFile().Contains("test.h"));

  CPPUNIT_ASSERT(!frame->OpenFile(
    wxExFileName(m_Project),
    0,
    wxEmptyString,
    wxExFrameWithHistory::WIN_IS_PROJECT));

  // It does not open, next should fail.
  CPPUNIT_ASSERT(!frame->GetRecentProject().Contains(m_Project));
  
  CPPUNIT_ASSERT( frame->SetRecentFile("xxx.cpp"));
  CPPUNIT_ASSERT(!frame->SetRecentProject("xxx.prj"));
  
  // frame->FindInFilesDialog(ID_TOOL_REPORT_FIND);
  CPPUNIT_ASSERT(!frame->GetFindInCaption(ID_TOOL_REPORT_FIND).empty());
  
  frame->FileHistoryPopupMenu();
  frame->ProjectHistoryPopupMenu();
  
  CPPUNIT_ASSERT(!frame->Grep("xxxxxxx"));
  CPPUNIT_ASSERT(!frame->Grep("xxxxxxx yyy"));
  CPPUNIT_ASSERT( frame->Grep("xxxxxxx ./ *.cpp"));
}
