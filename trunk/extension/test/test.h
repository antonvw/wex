/******************************************************************************\
* File:          test.h
* Purpose:       Declaration of classes for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXTESTFIXTURE_H
#define _EXTESTFIXTURE_H

#include <TestFixture.h>
#include <wx/extension/extension.h>
#include <wx/extension/dir.h>
#include <wx/extension/dir.h>
#include <wx/extension/svn.h>
#include <wx/extension/textfile.h>

class exTestSuite : public CppUnit::TestSuite
{
public:
  exTestSuite();
};

/// CppUnit test case.
class exTestFixture : public CppUnit::TestFixture
{
public:
  /// Constructor.
  exTestFixture() : TestFixture() {
    m_Config = NULL;
    m_Dir = NULL;
    m_File = NULL;
    m_FileName = NULL;
    m_FileNameStatistics = NULL;
    m_FindReplaceData = NULL;
    m_Lexer = NULL;
    m_Lexers = NULL;
    m_RCS = NULL;
    m_SVN = NULL;
    m_Stat = NULL;
    m_Statistics = NULL;
    m_Tool = NULL;
    };
    
 ~exTestFixture() {
    delete m_Config;
    delete m_Dir;
    delete m_File;
    delete m_FileName;
    delete m_FileNameStatistics;
    delete m_FindReplaceData;
    delete m_Lexer;
    delete m_Lexers;
    delete m_RCS;
    delete m_SVN;
    delete m_Stat;
    delete m_Statistics;
    delete m_Tool;
    };

  /// From TestFixture.
  /// Set up context before running a test. 
  virtual void setUp();
  
  /// Clean up after the test run.
  virtual void tearDown();

  /// Test the constructors of various extension classes.
  void testBaseConstructors();

  /// Test methods of various extension classes.
  void testBaseMethods();

  /// Test the constructors of various extension classes.
  void testConstructors();

  /// Test methods of various extension classes.
  void testMethods();
private:
  exConfig* m_Config; ///< testing exConfig
  exDir* m_Dir; ///< testing exDir
  exFile* m_File; ///< testing exFile
  exFileName* m_FileName; ///< testing exFileName
  exFileNameStatistics* m_FileNameStatistics; ///< testing exFileNameStatistics
  exFindReplaceData* m_FindReplaceData; ///< testing exFindReplaceData
  exLexer* m_Lexer; ///< testing exLexer
  exLexers* m_Lexers; ///< testing exLexers
  exRCS* m_RCS; ///< testing exRCS
  exStat* m_Stat; ///< testing exStat
  exStatistics<long>* m_Statistics; ///< testing exStatstics
  exSVN* m_SVN; ///< testing exSVN
  exTool* m_Tool; ///< testing exTool
};
#endif
