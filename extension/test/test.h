////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTESTUNIT_H
#define _EXTESTUNIT_H

#include <string>
#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <wx/stdpaths.h>
#include <wx/extension/filename.h>

//#define SHOW_REPORT

#define SETUP_ENV()                                            \
  const wxString dir(wxStandardPaths::Get().GetUserDataDir()); \
  if (!wxDirExists(dir))                                       \
  {                                                            \
    system("mkdir "+ dir);                                     \
    system("cp ../extension/data/*.xml " + dir);               \
  }                                                            \
    
/// CppUnit test fixture.
class wxExTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExTestFixture() 
    : TestFixture() 
    , m_TestDir("./")
    , m_TestFile(m_TestDir + "test.h"){}; 
  
  /// Destructor.
 ~wxExTestFixture() {};
 
  /// Returns the test dir.
  const wxString& GetTestDir() const {return m_TestDir;};
  
  /// Returns the test file.
  const wxExFileName& GetTestFile() const {return m_TestFile;};
  
  /// Clean up after the test run.
  /// Prints out report if switch is on.
  virtual void tearDown() {
#ifdef SHOW_REPORT
    if (!m_Report.empty()) 
      std::cout << m_Report;
#endif  
    };
      
  /// Adds text to report.
  void Report(const std::string& text) {
    m_Report.append(text);
    m_Report.append("\n");};
private:
  std::string m_Report;  
  const wxString m_TestDir;
  wxExFileName m_TestFile;
};
#endif
