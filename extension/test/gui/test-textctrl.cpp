////////////////////////////////////////////////////////////////////////////////
// Name:      test-textctrl.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/textctrl.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::textctrl")
{
  wxTextCtrl* tc = new wxTextCtrl(frame(), wxID_ANY);
  AddPane(frame(), tc);
  
  REQUIRE( wex::textctrl_input(wex::ex_command::type_t::NONE).get().empty());
  REQUIRE( wex::textctrl_input(wex::ex_command::type_t::NONE).values().empty());
  REQUIRE(!wex::textctrl_input(wex::ex_command::type_t::NONE).set(WXK_UP, tc));
  
  wex::textctrl_input tip(wex::ex_command::type_t::FIND);
  REQUIRE( tip.set("one"));
  REQUIRE(!tip.set(std::string()));
  REQUIRE( tip.get() == "one");
  REQUIRE( tip.values().front() == "one");
  
  tip.set(std::list < std::string > {"find3","find4","find5"});
  REQUIRE( tip.get() == "find3");
  REQUIRE( tip.values().size() == 3);
  
  tc->SetValue("hello");
  REQUIRE( tip.set(tc));
  REQUIRE( tip.get() == "hello");
  REQUIRE( tip.values().size() == 4);
  
  // Test keys.
  REQUIRE( tip.set(WXK_HOME));
  REQUIRE( tip.get() == "hello");
  REQUIRE( tip.set(WXK_END));
  REQUIRE( tip.get() == "find5");
  REQUIRE( tip.set(WXK_HOME));
  REQUIRE( tip.get() == "hello");
  REQUIRE( tip.set(WXK_DOWN));
  REQUIRE( tip.get() == "hello");
  REQUIRE( tip.set(WXK_PAGEDOWN));
  REQUIRE( tip.get() == "hello");

  REQUIRE(!tip.set(WXK_NONE, tc));

  tip.set(std::list < std::string > {"1","2", "3", "4", "5", "6", "7", "8",
    "9", "10", "11", "12"});
  for (auto key : std::vector<int> {WXK_UP, WXK_DOWN, WXK_HOME, WXK_END,
    WXK_PAGEUP, WXK_PAGEDOWN}) 
  {
    REQUIRE( tip.set(key, tc));
  }

  const std::list < std::string > e{};
  tip.set(e);
  REQUIRE( tip.values().empty());
}
