////////////////////////////////////////////////////////////////////////////////
// Name:      test-art.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/art.h>
#include "test.h"

TEST_CASE("wex::art")
{
  REQUIRE(!wex::stockart(0).GetBitmap().is_ok());
  REQUIRE(!wex::stockart(wxID_ANY).GetBitmap().is_ok());
  REQUIRE( wex::stockart(wxID_NEW).GetBitmap().is_ok());
  REQUIRE( wex::stockart(wxID_OPEN).GetBitmap().is_ok());
}
