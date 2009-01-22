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
  m_Stat = new exStat("test.h");
  m_Statistics = new exStatistics<long>(); 
  m_Tool = new exTool(ID_TOOL_REPORT_COUNT);
}

void exTestFixture::testConstructors()
{
  CPPUNIT_ASSERT(m_File != NULL);
}

void exTestFixture::testMethods() 
{
  // test exFile
  CPPUNIT_ASSERT(m_File->GetStat().IsOk());
  CPPUNIT_ASSERT(m_File->GetFileName().GetFullPath() == "test.h");

  // test exFile
  CPPUNIT_ASSERT(m_FileName->GetStat().IsOk());
  
  // test exLexer
  m_Lexer->SetLexerFromText("// this is a cpp comment text");
  CPPUNIT_ASSERT(m_Lexer->GetScintillaLexer().empty()); // we have no lexers
  
  // test exRCS
  CPPUNIT_ASSERT(m_RCS->GetUser().empty());
  
  // test exStat
  CPPUNIT_ASSERT(m_Stat->IsOk());
  
  // test exStatistics
  m_Statistics->Inc("test");
  CPPUNIT_ASSERT(m_Statistics->Get("test") == 1);
  m_Statistics->Inc("test");
  CPPUNIT_ASSERT(m_Statistics->Get("test") == 2);
  m_Statistics->Set("test", 13);
  CPPUNIT_ASSERT(m_Statistics->Get("test") == 13);
  
  // test exTool
  CPPUNIT_ASSERT(m_Tool->IsStatisticsType() > 0);
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
  m_SVN = new exSVN(SVN_STAT, "test.h"); 
}

void exAppTestFixture::testMethods() 
{
  // test exApp
  m_App->OnInit();
  
  CPPUNIT_ASSERT(m_App->GetConfig() != NULL);
  CPPUNIT_ASSERT(m_App->GetLexers() != NULL);
  CPPUNIT_ASSERT(m_App->GetPrinter() != NULL);
  
  // test exSVN
  m_SVN->Get();
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

/* TODO: the exApp or wxApp not yet okay without normal wxApp initialization

  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testConstructors", 
    &exAppTestFixture::testConstructors));
    
  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testMethods", 
    &exAppTestFixture::testMethods));
    */
}
