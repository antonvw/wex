/******************************************************************************\
* File:          test.h
* Purpose:       Declaration of classes for wxExtension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: test.h 410 2009-02-28 14:48:59Z antonvw $
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
#include <wx/extension/textfile.h>

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
    m_Config = NULL;
    m_File = NULL;
    m_FileName = NULL;
    m_FileNameStatistics = NULL;
    m_Lexer = NULL;
    m_Lexers = NULL;
    m_RCS = NULL;
    m_Stat = NULL;
    m_Statistics = NULL;
    };

  /// Destructor.
 ~wxExTestFixture() {
    delete m_Config;
    delete m_File;
    delete m_FileName;
    delete m_FileNameStatistics;
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

  /// Test the constructors of various extension classes.
  void testConstructors();

  /// Test methods of various extension classes.
  void testMethods();

  /// Test timing of methods.
  void testTiming();

  /// Test timing of methods.
  void testTimingAttrib();

  /// Test timing of methods.
  void testTimingConfig();
private:
  wxExConfig* m_Config; ///< testing wxExConfig
  wxExFile* m_File; ///< testing wxExFile
  wxExFileName* m_FileName; ///< testing wxExFileName
  wxExFileNameStatistics* m_FileNameStatistics; ///< testing wxExFileNameStatistics
  wxExLexer* m_Lexer; ///< testing wxExLexer
  wxExLexers* m_Lexers; ///< testing wxExLexers
  wxExRCS* m_RCS; ///< testing wxExRCS
  wxExStat* m_Stat; ///< testing wxExStat
  wxExStatistics<long>* m_Statistics; ///< testing wxExStatstics
};

#endif
