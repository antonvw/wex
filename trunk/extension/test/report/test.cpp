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
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_PRJ "./test-rep.prj"

void wxExReportAppTestFixture::setUp()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  m_ListView = new wxExListViewFile(frame, frame, TEST_FILE);
  m_Dir = new wxExDirWithListView(m_ListView, "./");
  m_Process = new wxExProcess(frame, "wc test.h");
  m_STC = new wxExSTCWithFrame(frame, frame, wxExFileName(TEST_FILE));
}

void wxExReportAppTestFixture::testConstructors()
{
}

void wxExReportAppTestFixture::testMethods()
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

  // test wxExDirWithListView
  CPPUNIT_ASSERT(m_Dir->FindFiles());

  // test wxExFrameWithHistory
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  CPPUNIT_ASSERT(!frame->OpenFile(wxExFileName(TEST_FILE))); // as we have no focused stc
  CPPUNIT_ASSERT(frame->GetRecentFile().Contains("test.h"));
  CPPUNIT_ASSERT(!frame->OpenFile(
    wxExFileName(TEST_PRJ),
    0,
    wxEmptyString,
    wxExSTCWithFrame::STC_WIN_IS_PROJECT));
  CPPUNIT_ASSERT(!frame->GetRecentProject().Contains("test-rep.prj"));

  // test wxExListViewFile
  // test wxExListView
  m_ListView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  m_ListView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));
  CPPUNIT_ASSERT(m_ListView->FindColumn("String") == 0);
  CPPUNIT_ASSERT(m_ListView->FindColumn("Number") == 1);
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

  // test wxExProcess
  CPPUNIT_ASSERT(m_Process->IsSelected());
  long pid;
  CPPUNIT_ASSERT((pid = m_Process->Execute()) > 0);
  CPPUNIT_ASSERT(wxProcess::Exists(pid));
//  CPPUNIT_ASSERT(m_Process->Kill() == wxKILL_OK);
//  CPPUNIT_ASSERT(!wxProcess::Exists(pid));

  // test wxExSTCWithFrame
  CPPUNIT_ASSERT(m_STC->GetFileName().GetFullPath().Contains("test.h"));
}

void wxExReportAppTestFixture::tearDown()
{
}

wxExReportTestSuite::wxExReportTestSuite()
  : CppUnit::TestSuite("wxexreport test suite")
{
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testConstructors",
    &wxExReportAppTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testMethods",
    &wxExReportAppTestFixture::testMethods));
}

