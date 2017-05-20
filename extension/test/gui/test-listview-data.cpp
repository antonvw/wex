////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview-data.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/listview-data.h>
#include <wx/extension/listview.h>
#include "test.h"

TEST_CASE("wxExListViewData")
{
  SUBCASE("Constructor")
  {
    REQUIRE( wxExListViewData().Image() == IMAGE_ART);
    REQUIRE( wxExListViewData().Type() == LIST_NONE);
    REQUIRE(!wxExListViewData().TypeDescription().empty());
    REQUIRE( wxExListViewData().Image(IMAGE_NONE).Image() == IMAGE_NONE);
    REQUIRE( wxExListViewData(wxExControlData().Col(3)).Control().Col() == 3);
    REQUIRE( wxExListViewData(wxExWindowData().Name("XX")).Window().Name() == "XX");
  }
  
  SUBCASE("Inject")
  {
    wxExListView* lv = new wxExListView();
    REQUIRE( wxExListViewData(lv).Inject());
    REQUIRE( wxExListViewData(lv, wxExControlData().Line(2)).Inject());
  }
}
