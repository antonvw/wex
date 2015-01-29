////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcfile.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/stcfile.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

void wxExGuiTestFixture::testSTCFile()
{
  wxExSTC* stc = new wxExSTC(m_Frame, GetTestFile());
  wxExSTCFile file(stc);

  // The file itself is not assigned.  
  CPPUNIT_ASSERT(!file.GetFileName().GetStat().IsOk());
  CPPUNIT_ASSERT(!file.GetContentsChanged());

  CPPUNIT_ASSERT( file.Read(GetTestDir() + "test.bin"));
  
  file.FileNew(wxExFileName("xxxx"));
}
