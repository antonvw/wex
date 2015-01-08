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
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExSTCWithFrame stc(frame, frame, GetTestFile());
  
  CPPUNIT_ASSERT(stc.GetFileName().GetFullPath().Contains("test.h"));
  
  stc.PropertiesMessage();
}
