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

#include <TestCaller.h>
#include "test.h"

void exTestFixture::setUp()
{
  m_File = new exFile("test.h");
  m_FileName = new exFileName("test.h");
  m_FileNameStatistics = new exFileNameStatistics("test.h");
  m_Lexer = new exLexer(); 
  m_RCS = new exRCS();  
  m_SVN = new exSVN(SVN_STAT); 
  m_Stat = new exStat("test.h");
  m_Statistics = new exStatistics<long>(); 
  m_Tool = new exTool(ID_TOOL_LINE);
}

void exTestFixture::testConstructors()
{
  assert(m_File != NULL);
}

void exTestFixture::testMethods() 
{
  assert(m_File->GetStat().IsOk());
  assert(m_FileName->GetStat().IsOk());
  assert(m_Stat->IsOk());
}

void exTestFixture::tearDown()
{
}
 
void exAppTestFixture::setUp()
{
}
 
void exAppTestFixture::testConstructors() 
{
  m_App = new exApp();
  m_Dir = new exDir("test.h");
}

void exAppTestFixture::testMethods() 
{
  m_App->OnInit();
  
  assert(m_App->GetConfig() != NULL);
  assert(m_App->GetLexers() != NULL);
  assert(m_App->GetPrinter() != NULL);
}

void exAppTestFixture::tearDown()
{
}
 
exTestSuite::exTestSuite()
  : CppUnit::TestSuite("wxextension test suite")

{
  // Add the tests.
  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testConstructors", 
    &exTestFixture::testConstructors));
    
  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testMethods", 
    &exTestFixture::testMethods));
    
  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testConstructors", 
    &exAppTestFixture::testConstructors));
    
  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testMethods", 
    &exAppTestFixture::testMethods));
}
