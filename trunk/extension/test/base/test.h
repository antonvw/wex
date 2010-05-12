/******************************************************************************\
* File:          test.h
* Purpose:       Declaration of classes for wxExtension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXTESTUNIT_H
#define _EXTESTUNIT_H

#include <TestFixture.h>
#include <TestSuite.h>
#include <wx/extension/extension.h>

/// CppUnit test suite.
class wxExTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  wxExTestSuite();
};

/// CppUnit base test fixture.
class wxExTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExTestFixture() : TestFixture() {
    m_File = NULL;
    m_FileName = NULL;
    m_FileStatistics = NULL;
    m_Lexer = NULL;
    m_Lexers = NULL;
    m_RCS = NULL;
    m_Stat = NULL;
    m_Statistics = NULL;
    };

  /// Destructor.
 ~wxExTestFixture() {
    delete m_File;
    delete m_FileName;
    delete m_FileStatistics;
    delete m_Lexer;
    delete m_Lexers;
    delete m_RCS;
    delete m_Stat;
    delete m_Statistics;
    };

  /// From TestFixture.
  /// Set up context before running a test.
  virtual void setUp();

  /// Clean up after the test run.
  virtual void tearDown();

  /// Test methods of various extension classes.
  void testMethods();

  /// Test timing of methods.
  void testTiming();

  /// Test timing of methods.
  void testTimingAttrib();
private:
  wxExFile* m_File; ///< testing wxExFile
  wxExFileName* m_FileName; ///< testing wxExFileName
  wxExFileStatistics* m_FileStatistics; ///< testing wxExFileStatistics
  wxExLexer* m_Lexer; ///< testing wxExLexer
  wxExLexers* m_Lexers; ///< testing wxExLexers
  wxExRCS* m_RCS; ///< testing wxExRCS
  wxExStat* m_Stat; ///< testing wxExStat
  wxExStatistics<long>* m_Statistics; ///< testing wxExStatstics
};

#endif
