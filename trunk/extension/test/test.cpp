/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <ui/text/TestRunner.h>
#include <TestSuite.h>
#include <TestCaller.h>
#include "test.h"

using CppUnit::Test;

void exTestFixture::testConstructors() 
{
  assert(m_File->GetStat().IsOk());
  assert(m_FileName->GetStat().IsOk());
  assert(m_Stat->IsOk());
}

void exTestFixture::testMethods() 
{
}

Test* exTestFixture::suite()
{
  CppUnit::TestSuite* testSuite = new CppUnit::TestSuite("wxextension test suite");
  
  // Add the tests.
  testSuite->addTest(new CppUnit::TestCaller<exTestFixture>(
    "testConstructors", 
    &exTestFixture::testConstructors));    
  testSuite->addTest(new CppUnit::TestCaller<exTestFixture>(
    "testMethods", 
    &exTestFixture::testMethods));
       
  return testSuite;
}

void exTestFixture::setUp()
{
  m_File = new exFile("test.h");
  m_FileName = new exFileName("test.h");
  m_Stat = new exStat("test.h");
}

void exTestFixture::tearDown()
{
  delete m_File;
  delete m_FileName;
  delete m_Stat;
} 

int main (int argc, char* argv[]) 
{
  CppUnit::TextUi::TestRunner runner;
  
  runner.addTest(exTestFixture::suite());
  runner.run();
  
  return 0;
}
