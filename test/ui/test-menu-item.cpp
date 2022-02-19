////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu-item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/file-history.h>
#include <wex/ui/menu-item.h>
#include <wex/ui/menu.h>

#include "test.h"

TEST_CASE("wex::menu_item")
{
  SUBCASE("default constructor")
  {
    REQUIRE(wex::menu_item().type() == wex::menu_item::SEPARATOR);
    REQUIRE(wex::menu_item().id() == -1);
    REQUIRE(wex::menu_item().name().empty());
  }

  SUBCASE("constructor MENU")
  {
    REQUIRE(
      wex::menu_item(wex::menu_item::HISTORY).type() ==
      wex::menu_item::HISTORY);
    REQUIRE(
      wex::menu_item(wex::menu_item::MENU).type() == wex::menu_item::MENU);
  }

  SUBCASE("constructor RADIO")
  {
    wex::menu_item item(10, "check", wex::menu_item::RADIO);

    REQUIRE(item.id() == 10);
    REQUIRE(item.name() == "check");
    REQUIRE(item.type() == wex::menu_item::RADIO);
  }

  SUBCASE("constructor SUBMENU")
  {
    wex::menu m;
    REQUIRE(wex::menu_item(&m, "submenu").type() == wex::menu_item::SUBMENU);
  }

  SUBCASE("constructor VCS")
  {
    REQUIRE(wex::menu_item(wex::path(), frame()).type() == wex::menu_item::VCS);
  }

  SUBCASE("constructor HISTORY")
  {
    wex::file_history fh;
    wex::menu_item    item(10, fh);

    REQUIRE(item.id() == 10);
    REQUIRE(item.type() == wex::menu_item::HISTORY);
  }

  SUBCASE("constructor PANES")
  {
    REQUIRE(wex::menu_item(frame()).type() == wex::menu_item::PANES);
  }
}
