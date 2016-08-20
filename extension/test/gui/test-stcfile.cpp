////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcfile.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
  stc->SetText("and still they came");
  
  AddPane(GetFrame(), stc);
  
  wxExSTCFile file(stc);

  // The file itself is not assigned.  
  REQUIRE(!file.GetFileName().GetStat().IsOk());
  REQUIRE(!file.GetContentsChanged());

  file.FileNew(wxExFileName("test-file.txt")); // clears stc document
  
  REQUIRE( stc->GetText().empty());
  stc->SetText("No, the game never ends "
    "when your whole world depends "
    "on the turn of a friendly card.");
  REQUIRE(!file.GetContentsChanged());
  REQUIRE( file.FileSave());
  REQUIRE(!file.GetContentsChanged());
  REQUIRE( remove("test-file.txt") == 0);
}
