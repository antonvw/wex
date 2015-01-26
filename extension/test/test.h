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
#include <wx/extension/filename.h>

/// Sets environment. 
void SetEnvironment(const wxString& dir);

/// Sets working directory to test dir, returns current working directory.
const wxString SetWorkingDirectory();

/// CppUnit test fixture.
class wxExTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExTestFixture(); 
  
  /// Destructor.
 ~wxExTestFixture() {};
 
  /// Returns the test dir.
  const wxString& GetTestDir() const {return m_TestDir;};
  
  /// Returns the test file.
  const wxExFileName& GetTestFile() const {return m_TestFile;};
  
  /// Clean up after the test run.
  /// Prints out report if switch is on.
  virtual void tearDown();
      
  /// Adds text to report.
  void Report(const std::string& text);
private:
  std::string m_Report;  
  const wxString m_TestDir;
  wxExFileName m_TestFile;
};
#endif
