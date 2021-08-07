////////////////////////////////////////////////////////////////////////////////
// Name:      test-grid-statistics.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/grid-statistics.h>

#include "test.h"

TEST_CASE("wex::grid_statistics")
{
  auto* statistics1 = new wex::grid_statistics<int>;
  auto* statistics2 = new wex::grid_statistics<int>;

  REQUIRE(statistics1->get().empty());
  REQUIRE(statistics1->get_items().empty());
  REQUIRE(statistics1->get("xx") == 0);
  REQUIRE(statistics1->get_items().empty());
  REQUIRE(statistics1->get_keys().empty());

  REQUIRE(statistics2->inc("xx") == 1);
  REQUIRE(statistics2->get_items().size() == 1);
  REQUIRE(statistics2->get_keys().size() == 1);
  REQUIRE(statistics2->set("xx", 3) == 3);
  REQUIRE(statistics2->get_items().size() == 1);
  REQUIRE(statistics2->get_keys().size() == 1);
  REQUIRE(statistics2->dec("xx") == 2);
  REQUIRE(statistics2->get_items().size() == 1);
  REQUIRE(statistics2->get_keys().size() == 1);
  REQUIRE(statistics2->inc("yy") == 1);
  REQUIRE(statistics2->get_items().size() == 2);
  REQUIRE(statistics2->get_keys().size() == 2);

  *statistics1 += *statistics2;
  REQUIRE(statistics1->get_items().size() == 2);
  REQUIRE(statistics1->get_keys().size() == 2);

  REQUIRE(statistics1->get("xx") == 2);

  REQUIRE(statistics1->GetNumberRows() == 2);
  auto* grid1 = statistics1->show();
  REQUIRE(statistics1->GetNumberRows() == 2);
  REQUIRE(statistics1->get_keys().size() == 2);
  REQUIRE(grid1 != nullptr);
  frame()->pane_add(grid1);

  auto* grid2 = statistics2->show();
  REQUIRE(statistics2->get_keys().size() == 2);
  frame()->pane_add(grid2);

  REQUIRE(statistics1->show() == grid1);
  REQUIRE(statistics2->set("xx", 10) == 10);

  statistics2->clear();
  REQUIRE(statistics2->inc("xx") == 1);
  REQUIRE(statistics2->get_items().size() == 1);
}
