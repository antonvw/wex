////////////////////////////////////////////////////////////////////////////////
// Name:      test-odbc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/grid.h>
#include <wex/odbc.h>
#include <wex/stc.h>

#include "test.h"

TEST_CASE("wex::odbc")
{
#if wexUSE_ODBC
  // Ensure we have a database and a table.
  if (system("mysql test < odbc-create.sql") != 0)
  {
    // if no mysql just quit
    return;
  }

  wex::config(_("Datasource")).set_first_of("Test");
  wex::config(_("User")).set();
  wex::config(_("Password")).set();

  wex::odbc odbc;

  REQUIRE(!odbc.get_version_info().get().empty());
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
#endif
}
