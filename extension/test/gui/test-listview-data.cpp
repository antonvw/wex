////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/listview-data.h>
#include <wx/extension/listview.h>
#include "test.h"

TEST_CASE("wex::listview_data")
{
  SUBCASE("Constructor")
  {
    REQUIRE( wex::listview_data().Image() == wex::IMAGE_ART);
    REQUIRE( wex::listview_data().Type() == wex::LISTVIEW_NONE);
    REQUIRE(!wex::listview_data().TypeDescription().empty());
    REQUIRE( wex::listview_data().Image(wex::IMAGE_NONE).Image() == wex::IMAGE_NONE);
    REQUIRE( wex::listview_data(wex::control_data().Col(3)).Control().Col() == 3);
    REQUIRE( wex::listview_data(wex::window_data().Name("XX")).Window().Name() == "XX");
  }
  
  SUBCASE("Inject")
  {
    wex::listview* lv = new wex::listview();
    AddPane(GetFrame(), lv);
    REQUIRE( wex::listview_data(lv).Inject());
    REQUIRE( wex::listview_data(lv, wex::control_data().Line(2)).Inject());
  }
}
