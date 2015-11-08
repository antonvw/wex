////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <cppunit/TestFixture.h>
#include <wx/extension/app.h>
#include <wx/extension/filename.h>

class wxExManagedFrame;

/// Adds managed pane.
void AddPane(wxExManagedFrame* frame, wxWindow* pane);

/// Sets environment. 
void SetEnvironment(const wxString& dir);

/// Sets working directory to test dir, returns current working directory.
const wxString SetWorkingDirectory();

/// Derive your application from wxExApp.
class wxExTestApp: public wxExApp
{
public:
  /// Constructor.
  wxExTestApp() {}
private:
  virtual int OnExit();
  virtual bool OnInit();
  virtual int OnRun();
};

/// CppUnit test fixture.
class wxExTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExTestFixture(); 
  
  /// Returns the test dir.
  const wxString& GetTestDir() const {return m_TestDir;};
  
  /// Returns the test file.
  const wxExFileName& GetTestFile() const {return m_TestFile;};
  
  /// Adds text to report.
  void Report(const std::string& text);
  
  /// Clean up after the test run.
  /// Prints out report if switch is on.
  virtual void tearDown();
private:
  std::string m_Report;  
  const wxString m_TestDir;
  wxExFileName m_TestFile;
};
