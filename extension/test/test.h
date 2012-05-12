////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTESTUNIT_H
#define _EXTESTUNIT_H

#include <wx/extension/extension.h>

#if wxUSE_UNIX

#include <TestCaller.h>
#include <TestFixture.h>
#include <TestSuite.h>

/// CppUnit test fixture.
/// These classes require either an wxExApp object, or wx to be initialized.
class wxExTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExTestFixture() : TestFixture() {}; 
  
  /// Destructor.
 ~wxExTestFixture() {};
 
  /// Adds text to report.
  void Report(const wxString& text) {
    m_Report << text << "\n";};
 
  /// Prints out report.
  static void PrintReport() {
    if (!m_Report.empty()) 
      std::cout << m_Report;};
private:
  static wxString m_Report;  
};

wxString wxExTestFixture::m_Report;

#endif
#endif
