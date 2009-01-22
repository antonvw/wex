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

#include <TestSuite.h>
#include <TestCaller.h>
#include "test.h"

using CppUnit::Test;

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
  m_Tool = new exTool(100); // TODO: which number, and document in constructor  
}

void exTestFixture::testBaseConstructors() 
{
}

void exTestFixture::testBaseMethods() 
{
  assert(m_File->GetStat().IsOk());
  assert(m_FileName->GetStat().IsOk());
  assert(m_Stat->IsOk());
}

void exTestFixture::testConstructors() 
{
//  m_Config = new exConfig();
  m_Dir = new exDir("test.h");
//  m_FindReplaceData = new exFindReplaceData(m_Config);
  m_Lexers = new exLexers(); 
  m_RCS = new exRCS();  
  m_SVN = new exSVN(SVN_STAT); 
  m_Stat = new exStat("test.h");
  m_Statistics = new exStatistics<long>(); 
  m_Tool = new exTool(100); // TODO: which number, and document in constructor  
}

void exTestFixture::testMethods() 
{
}

void exTestFixture::tearDown()
{
}
 
exTestSuite::exTestSuite()
  : CppUnit::TestSuite("wxextension test suite")

{
  // Add the tests.
  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testBaseConstructors", 
    &exTestFixture::testBaseConstructors));
    
  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testBaseMethods", 
    &exTestFixture::testBaseMethods));
/*    
  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testConstructors", 
    &exTestFixture::testConstructors));
    
  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testMethods", 
    &exTestFixture::testMethods));
*/       
}
