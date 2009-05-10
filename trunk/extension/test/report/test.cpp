/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxextension report cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: test.cpp 589 2009-04-09 13:43:53Z antonvw $
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
  m_Frame = new wxExFrameWithHistory(wxTheApp->GetTopWindow(), wxID_ANY, "wxExRep");
  m_ListView = new wxExListViewFile(m_Frame, wxExListViewFile::LIST_PROCESS);
  m_Dir = new wxExDirWithReport(m_ListView, "./");
  m_Process = new wxExProcessWithListView(m_ListView, "wc test.h");
  m_STC = new wxExSTCWithFrame(m_Frame, wxExFileName("test.h"));
}

void wxExReportAppTestFixture::testConstructors()
{
}

void wxExReportAppTestFixture::testMethods()
{
  // test wxExDirWithReport
  CPPUNIT_ASSERT(m_Dir->FindFiles());

  // test wxExFrameWithHistory
  CPPUNIT_ASSERT(m_Frame->OpenFile(wxExFileName("test.h")));
  CPPUNIT_ASSERT(m_Frame->GetRecentFile().Contains("test.h"));
  CPPUNIT_ASSERT(m_Frame->OpenFile(
    wxExFileName("test.h"),
    0,
    wxEmptyString,
    wxExSTCWithFrame::STC_OPEN_IS_PROJECT));
  CPPUNIT_ASSERT(m_Frame->GetRecentProject().Contains("test.h"));

  // test wxExListViewFile
  CPPUNIT_ASSERT(m_ListView->FileOpen(wxExFileName("test.prj")));
  CPPUNIT_ASSERT(m_ListView->ItemFromText("test1\ntest2\n"));

  // test wxExProcessWithListView
  CPPUNIT_ASSERT(m_Process->Run());
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

