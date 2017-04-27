////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc-data.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/stc-data.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExSTCData")
{
  wxExSTC* stc = GetSTC();

  SUBCASE("Constructor")
  {
    REQUIRE( wxExSTCData().Line() == 0);
    REQUIRE( wxExSTCData().Col(3).Col() == 3);
    REQUIRE( wxExSTCData().Line(-1).Line() == -1);
    REQUIRE( wxExSTCData().Flags(STC_WIN_READ_ONLY).Flags() == STC_WIN_READ_ONLY);
    REQUIRE( wxExSTCData().Flags(STC_WIN_READ_ONLY).Flags(STC_WIN_HEX, DATA_OR).
      Flags() != STC_WIN_READ_ONLY);
    REQUIRE( wxExSTCData(stc).Line(-1).Line() == 1);
    REQUIRE( wxExSTCData().Line(3).Line() == 3);
    REQUIRE( wxExSTCData().Find("xx").Find() == "xx");
    REQUIRE((wxExSTCData().Menu() & STC_MENU_VCS));
  }
  
  SUBCASE("Inject")
  {
    stc->SetText("line 1\nline 2\nline 3\n");
    REQUIRE( wxExSTCData(stc).Line(1).Col(5).Inject());
    REQUIRE( stc->GetCurrentLine() == 0);
    REQUIRE( stc->GetCurrentPos() == 4);
    REQUIRE(!wxExSTCData().Line(3).Col(3).Inject());
    REQUIRE( wxExSTCData(stc).Line(3).Col(3).Inject());
    REQUIRE( stc->GetCurrentLine() == 2);
    REQUIRE( stc->GetColumn(stc->GetCurrentPos()) == 2);
  }
}
