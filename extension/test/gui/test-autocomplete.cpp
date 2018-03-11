////////////////////////////////////////////////////////////////////////////////
// Name:      test-autocomplete.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/autocomplete.h>
#include "test.h"

TEST_CASE("wxExAutoComplete")
{
  wxExAutoComplete ac(GetSTC());

  REQUIRE(!ac.Activate(std::string()));
  REQUIRE(!ac.Apply(WXK_BACK));
  REQUIRE( ac.Apply('x'));
  REQUIRE( ac.Apply(WXK_BACK));
  REQUIRE(!ac.Apply(WXK_BACK));

  ac.Use(false);
  REQUIRE(!ac.Apply('x'));

  ac.Reset();
}
