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
    REQUIRE( wxExSTCData().Control().Line() == 0);
    REQUIRE( wxExSTCData().Control(wxExControlData().Col(3)).Control().Col() == 3);
    REQUIRE( wxExSTCData(wxExControlData().Col(3)).Control().Col() == 3);
    REQUIRE( wxExSTCData(wxExWindowData().Name("XX")).Window().Name() == "XX");
    REQUIRE( wxExSTCData().Flags(STC_WIN_READ_ONLY).Flags() == STC_WIN_READ_ONLY);
    REQUIRE( wxExSTCData().Flags(STC_WIN_READ_ONLY).Flags(STC_WIN_HEX, DATA_OR).
      Flags() != STC_WIN_READ_ONLY);
    REQUIRE((wxExSTCData().Menu() & STC_MENU_VCS));
  }
  
  SUBCASE("Inject")
  {
    stc->SetText("line 1\nline 2\nline 3\n");
    REQUIRE( wxExSTCData(stc).Control(wxExControlData().Line(1).Col(5)).Inject());
    REQUIRE( stc->GetCurrentLine() == 0);
    REQUIRE( stc->GetCurrentPos() == 4);
    REQUIRE( wxExSTCData(stc, wxExControlData().Line(1).Col(5)).Inject());
    REQUIRE(!wxExSTCData().Control(wxExControlData().Line(1).Col(5)).Inject());
  }
}
