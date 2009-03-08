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

void ftAppTestFixture::testConstructors()
{
}

void ftAppTestFixture::testMethods()
{
  // test ftSTC
  CPPUNIT_ASSERT(m_STC->GetFileName().GetFullName() == "test.h");
}

void ftAppTestFixture::tearDown()
{
}

ftTestSuite::ftTestSuite()
  : CppUnit::TestSuite("wxfiletool test suite")
{
  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testConstructors",
    &ftAppTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testMethods",
    &ftAppTestFixture::testMethods));
}

