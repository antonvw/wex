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
#include <wx/extension/art.h>
#include "test.h"

TEST_CASE("wex::art")
{
  REQUIRE(!wex::stockart(0).GetBitmap().IsOk());
  REQUIRE(!wex::stockart(wxID_ANY).GetBitmap().IsOk());
  REQUIRE( wex::stockart(wxID_NEW).GetBitmap().IsOk());
  REQUIRE( wex::stockart(wxID_OPEN).GetBitmap().IsOk());
}
