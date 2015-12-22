////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcfile.cpp
// Purpose:   Implementation for wxExtension unit testing
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

TEST_CASE("wxExSTCFile", "[stc]")
{
  wxExSTC* stc = new wxExSTC(GetFrame(), GetTestFile());
  
  AddPane(GetFrame(), stc);
  
  wxExSTCFile file(stc);

  // The file itself is not assigned.  
  REQUIRE(!file.GetFileName().GetStat().IsOk());
  REQUIRE(!file.GetContentsChanged());

  file.FileNew(wxExFileName("xxxx"));
}
