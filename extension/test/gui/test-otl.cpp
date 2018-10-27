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
  wex::config(_("User")).set("");
  wex::config(_("Password")).set("");
  
  wex::otl otl;
  
  REQUIRE(!otl.VersionInfo().Get().empty());
  REQUIRE(!otl.Datasource().empty());
  
  otl.Logon(wex::window_data().Button(0));

  bool stopped = false;

  wex::grid* grid = new wex::grid();
  AddPane(GetFrame(), grid);
  
  if (!otl.IsConnected())
  {
    REQUIRE( otl.Query("select * from one") == 0);
    REQUIRE( otl.Query("select * from one", GetSTC(), stopped) == 0);
    REQUIRE( otl.Query("select * from one", grid, stopped) == 0);
    REQUIRE(!otl.Logoff());
  }
  else
  {
    REQUIRE( otl.Query("select * from one") == 9);
    REQUIRE( otl.Logoff());
  }
#endif
}
