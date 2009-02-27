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

#ifndef _EXTESTUNIT_H
#define _EXTESTUNIT_H

#include <TestFixture.h>
#include <TestSuite.h>
#include <wx/extension/extension.h>
#include <wx/extension/dir.h>
#include <wx/extension/svn.h>
#include <wx/extension/textfile.h>

/// CppUnit test suite.
class exTestSuite : public CppUnit::TestSuite
{
public:
  /// Default constructor.
  exTestSuite();
};

/// CppUnit base test fixture.
class exTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  exTestFixture() : TestFixture() {
    m_Config = NULL;
    m_File = NULL;
    m_FileName = NULL;
    m_FileNameStatistics = NULL;
    m_Lexer = NULL;
    m_Lexers = NULL;
    m_RCS = NULL;
    m_Stat = NULL;
    m_Statistics = NULL;
    m_TextFile = NULL;
    m_Tool = NULL;
    };

  /// Destructor.
 ~exTestFixture() {
    delete m_Config;
    delete m_File;
    delete m_FileName;
    delete m_FileNameStatistics;
    delete m_Lexer;
    delete m_Lexers;
    delete m_RCS;
    delete m_Stat;
    delete m_Statistics;
    delete m_TextFile;
    delete m_Tool;
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
  exConfig* m_Config; ///< testing exConfig
  exFile* m_File; ///< testing exFile
  exFileName* m_FileName; ///< testing exFileName
  exFileNameStatistics* m_FileNameStatistics; ///< testing exFileNameStatistics
  exLexer* m_Lexer; ///< testing exLexer
  exLexers* m_Lexers; ///< testing exLexers
  exRCS* m_RCS; ///< testing exRCS
  exStat* m_Stat; ///< testing exStat
  exStatistics<long>* m_Statistics; ///< testing exStatstics
  exTextFile* m_TextFile; ///< testing exTextFile
  exTool* m_Tool; ///< testing exTool
};

/// Derive your application from exApp.
class exTestApp: public exApp
{
public:
  /// Constructor.
  exTestApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
};

/// CppUnit app test fixture.
/// These classes require either an exApp object, or wx to be initialized.
class exAppTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  exAppTestFixture() : TestFixture() {
    m_App = NULL;
    m_Dir = NULL;
    m_SVN = NULL;
    };

  /// Destructor.
 ~exAppTestFixture() {
    delete m_App;
    delete m_Dir;
    delete m_SVN;
    };

  /// From TestFixture.
  /// Set up context before running a test.
  virtual void setUp();

  /// Clean up after the test run.
  virtual void tearDown();

  /// Test the constructors of various extension classes.
  void testConstructors();

  /// Test methods of various extension classes requiring app.
  void testMethods();
private:
  exTestApp* m_App; ///< testing exApp
  exDir* m_Dir;     ///< testing exDir
  exSVN* m_SVN;     ///< testing exSVN
};
#endif
