////////////////////////////////////////////////////////////////////////////////
// Name:      test-art.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/art.h>
#include "test.h"

TEST_CASE("wxExArt")
{
  REQUIRE(!wxExStockArt(0).GetBitmap().IsOk());
  REQUIRE(!wxExStockArt(wxID_ANY).GetBitmap().IsOk());
  REQUIRE( wxExStockArt(wxID_NEW).GetBitmap().IsOk());
  REQUIRE( wxExStockArt(wxID_OPEN).GetBitmap().IsOk());
}
