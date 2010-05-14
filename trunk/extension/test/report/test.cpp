/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxExtension report cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <TestCaller.h>
#include <wx/config.h>
#include <wx/extension/report/report.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_PRJ "./test-rep.prj"

void wxExReportAppTestFixture::testConfig()
{
  wxConfig* cfg = new wxConfig(wxEmptyString, wxEmptyString, "test.cfg", wxEmptyString, wxCONFIG_USE_LOCAL_FILE);
  const int max = 100000;

  wxStopWatch sw;

  sw.Start();

  for (int j = 0; j < max; j++)
  {
    cfg->Read("test", 0l);
  }

  const long config = sw.Time();

  printf("wxConfig::Read:%ld\n", config);

}

void wxExReportAppTestFixture::testDirWithListView()
{
  wxExDirWithListView* m_Dir;
  m_Dir = new wxExDirWithListView(m_ListView, "./");
  CPPUNIT_ASSERT(m_Dir->FindFiles());
}

void wxExReportAppTestFixture::testFrameWithHistory()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  CPPUNIT_ASSERT(!frame->OpenFile(wxExFileName(TEST_FILE))); // as we have no focused stc
  CPPUNIT_ASSERT(frame->GetRecentFile().Contains("test.h"));
  CPPUNIT_ASSERT(!frame->OpenFile(
    wxExFileName(TEST_PRJ),
    0,
    wxEmptyString,
    wxExSTCWithFrame::STC_WIN_IS_PROJECT));
  CPPUNIT_ASSERT(!frame->GetRecentProject().Contains("test-rep.prj"));
}

void wxExReportAppTestFixture::testListViewFile()
{
  wxExListViewFile* m_ListView;
  m_ListView = new wxExListViewFile(frame, frame, TEST_FILE);
  // Remember that listview file already has columns.
  m_ListView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  m_ListView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));
  CPPUNIT_ASSERT(m_ListView->FindColumn("String") > 1);
  CPPUNIT_ASSERT(m_ListView->FindColumn("Number") > 1);
/*
  wxExListItem item1(m_ListView, "c item"); ///< testing wxExListItem
  item1.Insert();
  wxExListItem item2(m_ListView, "b item"); ///< testing wxExListItem
  item2.Insert();
  wxExListItem item3(m_ListView, "a item"); ///< testing wxExListItem
  item3.Insert();
  m_ListView->SortColumn("String", SORT_ASCENDING);
  wxExListItem test(m_ListView, 0);
  CPPUNIT_ASSERT(test.GetColumnText("String") == "a item");
*/

  CPPUNIT_ASSERT(m_ListView->FileLoad(TEST_PRJ));
  CPPUNIT_ASSERT(m_ListView->ItemFromText("test1\ntest2\n"));
}

void wxExReportAppTestFixture::testProcess()
{
  wxExProcess* m_Process;
  m_Process = new wxExProcess(frame, "wc test.h");
  CPPUNIT_ASSERT(m_Process->IsSelected());
  long pid = m_Process->Execute();
  // CPPUNIT_ASSERT(wxProcess::Exists(pid));
}

void wxExReportAppTestFixture::testSTCWithFrame()
{
  wxExSTCWithFrame* m_STC;
  m_STC = new wxExSTCWithFrame(frame, frame, wxExFileName(TEST_FILE));
  CPPUNIT_ASSERT(m_STC->GetFileName().GetFullPath().Contains("test.h"));
}
  
wxExReportTestSuite::wxExReportTestSuite()
  : CppUnit::TestSuite("wxexreport test suite")
{
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testMethods",
    &wxExReportAppTestFixture::testMethods));
}
