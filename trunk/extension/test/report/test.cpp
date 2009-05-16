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

void wxExReportAppTestFixture::setUp()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  m_ListView = new wxExListViewFile(frame, wxExListViewFile::LIST_PROCESS);
  m_Dir = new wxExDirWithListView(m_ListView, "./");
  m_Process = new wxExProcessWithListView(m_ListView, "wc test.h");
  m_STC = new wxExSTCWithFrame(frame, wxExFileName("test.h"));
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
  CPPUNIT_ASSERT(!frame->OpenFile(wxExFileName("test.h"))); // as we have no focused stc
  CPPUNIT_ASSERT(frame->GetRecentFile().Contains("test.h"));
  CPPUNIT_ASSERT(!frame->OpenFile(
    wxExFileName("test.prj"),
    0,
    wxEmptyString,
    wxExSTCWithFrame::STC_OPEN_IS_PROJECT));
  CPPUNIT_ASSERT(!frame->GetRecentProject().Contains("test.prj"));

  // test wxExListViewFile
  CPPUNIT_ASSERT(m_ListView->FileOpen(wxExFileName("test.prj")));
  CPPUNIT_ASSERT(m_ListView->ItemFromText("test1\ntest2\n"));

  // test wxExProcessWithListView
  CPPUNIT_ASSERT(m_Process->Execute());
  CPPUNIT_ASSERT(m_Process->Kill() == wxKILL_OK);
  CPPUNIT_ASSERT(m_Process->Execute()); // repeat test
  CPPUNIT_ASSERT(m_Process->Kill() == wxKILL_OK);
  
  // test wxExSTCWithFrame
  CPPUNIT_ASSERT(m_STC->GetFileName().GetFullName() == "test.h");
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

