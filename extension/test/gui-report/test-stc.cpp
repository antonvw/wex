////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/stc.h>
#include "test.h"

void wxExGuiReportTestFixture::testSTCWithFrame()
{
  wxExSTCWithFrame stc(m_Frame, m_Frame, GetTestFile());
  
  CPPUNIT_ASSERT( stc.GetFileName().GetFullPath().Contains("test.h"));
  CPPUNIT_ASSERT( stc.Open(GetTestFile()));
  CPPUNIT_ASSERT(!stc.Open(wxExFileName("XXX")));
  
  stc.PropertiesMessage();
}
