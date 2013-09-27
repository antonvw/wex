////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTESTUNIT_H
#define _EXTESTUNIT_H

#include <string>
#include <TestCaller.h>
#include <TestFixture.h>
#include <TestSuite.h>

//#define SHOW_REPORT

/// CppUnit test fixture.
class wxExTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExTestFixture() : TestFixture() {}; 
  
  /// Destructor.
 ~wxExTestFixture() {};
 
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
};
#endif
