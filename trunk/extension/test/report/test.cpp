/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxextension report cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <TestCaller.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_PRJ "./test-rep.prj"

void wxExReportAppTestFixture::setUp()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  m_ListView = new wxExListViewWithFrame(frame, wxExListViewWithFrame::LIST_PROCESS);
  m_Dir = new wxExDirWithListView(m_ListView, "./");
  m_Process = new wxExProcessWithListView(frame, m_ListView, "wc test.h");
  m_STC = new wxExSTCWithFrame(frame, wxExFileName(TEST_FILE));
}

void wxExReportAppTestFixture::testConstructors()
{
}

void wxExReportAppTestFixture::testMethods()
{
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
    wxExSTCWithFrame::STC_OPEN_IS_PROJECT));
  CPPUNIT_ASSERT(!frame->GetRecentProject().Contains("test-rep.prj"));

  // test wxExListViewWithFrame
  CPPUNIT_ASSERT(m_ListView->FileLoad(wxExFileName(TEST_PRJ)));
  CPPUNIT_ASSERT(m_ListView->ItemFromText("test1\ntest2\n"));

  // test wxExProcessWithListView
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

