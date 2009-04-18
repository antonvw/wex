/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxfiletool cpp unit testing
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

void ftAppTestFixture::setUp()
{
  m_ListView = new exListViewFile(wxTheApp->GetTopWindow(), exListViewFile::LIST_PROCESS);
  m_Dir = new exDirWithReport(m_ListView, "./");
  m_Process = new exProcessWithListView(m_ListView, "wc test.h");
  m_STC = new exSTCWithFrame(wxTheApp->GetTopWindow(), exFileName("test.h"));
}

void ftAppTestFixture::testConstructors()
{
}

void ftAppTestFixture::testMethods()
{
  // test exDirWithReport
  CPPUNIT_ASSERT(m_Dir->FindFiles());

  // test exListViewFile
  CPPUNIT_ASSERT(m_ListView->FileOpen(exFileName("test.prj")));
  CPPUNIT_ASSERT(m_ListView->ItemFromText("test1\ntest2\n"));

  // test exProcessWithListView
  CPPUNIT_ASSERT(m_Process->Run());

  // test exSTCWithFrame
  CPPUNIT_ASSERT(m_STC->GetFileName().GetFullName() == "test.h");
}

void ftAppTestFixture::tearDown()
{
}

ftTestSuite::ftTestSuite()
  : CppUnit::TestSuite("wxfiletool test suite")
{
  addTest(new CppUnit::TestCaller<ftAppTestFixture>(
    "testConstructors",
    &ftAppTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<ftAppTestFixture>(
    "testMethods",
    &ftAppTestFixture::testMethods));
}

