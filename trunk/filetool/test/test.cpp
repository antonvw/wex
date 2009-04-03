/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxfiletool cpp unit testing
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

void ftAppTestFixture::setUp()
{
  m_ListView = new ftListView(wxTheApp->GetTopWindow(), ftListView::LIST_PROCESS);
  m_Dir = new ftDir(m_ListView, "./");
  m_Process = new ftProcess(m_ListView, "wc test.h");
  m_STC = new ftSTC(wxTheApp->GetTopWindow(), exFileName("test.h"));
}

void ftAppTestFixture::testConstructors()
{
}

void ftAppTestFixture::testMethods()
{
  // test ftDir
  CPPUNIT_ASSERT(m_Dir->FindFiles());

  // test ftListView
  CPPUNIT_ASSERT(m_ListView->FileOpen(exFileName("test.prj")));
  CPPUNIT_ASSERT(m_ListView->ItemFromText("test1\ntest2\n"));

  // test ftProcess
  CPPUNIT_ASSERT(m_Process->Run());

  // test ftSTC
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

