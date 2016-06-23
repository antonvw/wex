////////////////////////////////////////////////////////////////////////////////
// Name:      test-textctrl.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/textctrl.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExTextCtrl")
{
  wxTextCtrl* tc = new wxTextCtrl(GetFrame(), wxID_ANY);
  AddPane(GetFrame(), tc);
  
  REQUIRE( wxExTextCtrlInput("xxx").Get().empty());
  REQUIRE( wxExTextCtrlInput("xxx").GetValues().empty());
  REQUIRE(!wxExTextCtrlInput("xxx").Set(WXK_UP, tc));
  
  wxExTextCtrlInput tip("tip");
  REQUIRE( tip.Set("one"));
  REQUIRE(!tip.Set(wxString()));
  REQUIRE( tip.Get() == "one");
  REQUIRE( tip.GetValues().front() == "one");
  
  tip.Set(std::list < wxString > {"find3","find4","find5"});
  REQUIRE( tip.Get() == "find3");
  REQUIRE( tip.GetValues().size() == 3);
  
  tc->SetValue("hello");
  REQUIRE( tip.Set(tc));
  REQUIRE( tip.Get() == "hello");
  REQUIRE( tip.GetValues().size() == 4);
  
  // Test keys.
  REQUIRE( tip.Set(WXK_HOME));
  REQUIRE( tip.Get() == "hello");
  REQUIRE( tip.Set(WXK_END));
  REQUIRE( tip.Get() == "find5");
  REQUIRE( tip.Set(WXK_HOME));
  REQUIRE( tip.Get() == "hello");
  REQUIRE( tip.Set(WXK_DOWN));
  REQUIRE( tip.Get() == "hello");
  REQUIRE( tip.Set(WXK_PAGEDOWN));
  REQUIRE( tip.Get() == "hello");

  REQUIRE(!tip.Set(WXK_NONE, tc));

  tip.Set(std::list < wxString > {"1","2", "3", "4", "5", "6", "7", "8",
    "9", "10", "11", "12"});
  for (auto key : std::vector<int> {WXK_UP, WXK_DOWN, WXK_HOME, WXK_END,
    WXK_PAGEUP, WXK_PAGEDOWN}) 
  {
    INFO( key);
    REQUIRE( tip.Set(key, tc));
  }
  
  const std::list < wxString > e{};
  tip.Set(e);
  REQUIRE( tip.GetValues().empty());
}
