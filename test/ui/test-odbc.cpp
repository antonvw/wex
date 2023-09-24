////////////////////////////////////////////////////////////////////////////////
// Name:      test-odbc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/ui/grid.h>
#include <wex/ui/odbc.h>

#include "test.h"

#if wexUSE_ODBC
TEST_CASE("wex::odbc")
{
  SUBCASE("static")
  {
    REQUIRE(!wex::odbc::get_version_info().get().empty());
  }

  SUBCASE("prep")
  {
    // Ensure we have a database and a table.
    if (system("mysql test < odbc-create.sql") != 0)
    {
      // if no mysql or error just quit
      return;
    }
  }

  SUBCASE("rest")
  {
    wex::config(_("Datasource")).set_first_of("Test");
    wex::config(_("User")).set();
    wex::config(_("Password")).set();

    wex::odbc odbc;

    REQUIRE(!odbc.datasource().empty());

    odbc.logon(wex::data::window().button(0));

    auto* grid = new wex::grid();
    frame()->pane_add(grid);

    if (!odbc.is_connected())
    {
      bool stopped = false;
      REQUIRE(odbc.query("select * from one") == 0);
      REQUIRE(odbc.query("select * from one", get_stc(), stopped) == 0);
      REQUIRE(odbc.query("select * from one", grid, stopped) == 0);
      REQUIRE(!odbc.logoff());
    }
    else
    {
      REQUIRE(odbc.query("select * from one") == 9);
      REQUIRE(odbc.logoff());
    }
  }
}
#endif
