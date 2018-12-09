////////////////////////////////////////////////////////////////////////////////
// Name:      test-otl.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/managedframe.h>
#include "test.h"
#include <wex/config.h>
#include <wex/grid.h>
#include <wex/otl.h>
#include <wex/stc.h>

TEST_CASE("wex::otl")
{
#if wexUSE_OTL
  // Ensure we have a database and a table.
  if (system("mysql test < otl-create.sql") != 0)
  {
    // if no mysql just quit
    return;
  }

  wex::config(_("Datasource")).set("Test");
  wex::config(_("User")).set();
  wex::config(_("Password")).set();
  
  wex::otl otl;
  
  REQUIRE(!otl.get_version_info().get().empty());
  REQUIRE(!otl.datasource().empty());
  
  otl.logon(wex::window_data().button(0));

  bool stopped = false;

  wex::grid* grid = new wex::grid();
  add_pane(frame(), grid);
  
  if (!otl.is_connected())
  {
    REQUIRE( otl.query("select * from one") == 0);
    REQUIRE( otl.query("select * from one", get_stc(), stopped) == 0);
    REQUIRE( otl.query("select * from one", grid, stopped) == 0);
    REQUIRE(!otl.logoff());
  }
  else
  {
    REQUIRE( otl.query("select * from one") == 9);
    REQUIRE( otl.logoff());
  }
#endif
}
