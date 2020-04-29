////////////////////////////////////////////////////////////////////////////////
// Name:      test-statistics.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/statistics.h>
#include <wex/managedframe.h>
#include "../test.h"

TEST_CASE("wex::statistics")
{
  auto* statistics1 = new wex::statistics<int>;
  auto* statistics2 = new wex::statistics<int>;
  
  REQUIRE(statistics1->get().empty());
  REQUIRE(statistics1->get_items().empty());
  REQUIRE(statistics1->get("xx") == 0);
  
  REQUIRE(statistics2->inc("xx") == 1);
  REQUIRE(statistics2->set("xx", 3) == 3);
  REQUIRE(statistics2->dec("xx") == 2);

  *statistics1 += *statistics2;
  
  REQUIRE(!statistics1->get().empty());
  REQUIRE( statistics1->get("xx") == 2);

  wex::grid* grid1 = statistics1->show(frame());
  REQUIRE(grid1 != nullptr);
  REQUIRE(grid1 == statistics1->get_grid());
  wex::test::add_pane(frame(), grid1);
  
  wex::grid* grid2 = statistics2->show(frame());
  wex::test::add_pane(frame(), grid2);
  
  REQUIRE(statistics1->show(frame()) == grid1);
  REQUIRE(statistics2->set("xx", 10) == 10);
  
  statistics2->clear();
  REQUIRE(statistics2->inc("xx") == 1);
  
  wxPostEvent(grid2, wxCommandEvent(wxEVT_MENU, wxID_CLEAR));
}
