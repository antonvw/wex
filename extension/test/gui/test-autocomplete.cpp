////////////////////////////////////////////////////////////////////////////////
// Name:      test-autocomplete.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/autocomplete.h>
#include "test.h"

TEST_CASE("wex::autocomplete")
{
  wex::autocomplete ac(get_stc());

  REQUIRE(!ac.activate(std::string()));
  REQUIRE(!ac.apply(WXK_BACK));
  REQUIRE( ac.apply('x'));
  REQUIRE( ac.apply(WXK_BACK));
  REQUIRE(!ac.apply(WXK_BACK));

  ac.use(false);
  REQUIRE(!ac.apply('x'));

  ac.reset();
}
