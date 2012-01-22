////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <TestCaller.h>
#include <wx/extension/test.h>

wxString wxExTestFixture::m_Report;  

void wxExTestFixture::PrintReport()
{
  if (!m_Report.empty())
  {
    std::cout << m_Report;
  }
}

void wxExTestFixture::Report(const wxString& text)
{
  m_Report << text << "\n";
}
