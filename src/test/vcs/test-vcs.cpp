////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/vcs.h>
#include <wex/config.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include "../test.h"

TEST_SUITE_BEGIN("wex::vcs");

TEST_CASE("wex::vcs")
{
  wex::path file(wex::test::get_path("test.h"));
  file.make_absolute();
  
  SUBCASE("statics")
  {
    REQUIRE( wex::vcs::dir_exists(file));
    REQUIRE( wex::vcs::load_document());
    REQUIRE( wex::vcs::size() > 0);
  }
  
  SUBCASE("others")
  {
    // In wex::app the vcs is loaded, so current vcs is known,
    // using this constructor results in command id 3, being add.
    wex::vcs vcs(std::vector< wex::path >{file}, 3);
    
    REQUIRE( vcs.config_dialog(wex::window_data().button(wxAPPLY | wxCANCEL)));
    
#ifndef __WXMSW__
    REQUIRE( vcs.execute());
    REQUIRE( vcs.execute("status"));
    REQUIRE(!vcs.execute("xxx"));
    
    REQUIRE( vcs.show_dialog(wex::window_data().button(wxAPPLY | wxCANCEL)));
    
    REQUIRE( vcs.request(wex::window_data().button(wxAPPLY | wxCANCEL)));

    REQUIRE( vcs.entry().build_menu(100, new wex::menu("test", 0)) > 0);
    REQUIRE( vcs.entry().get_command().get_command() == "add");
    
    REQUIRE(!vcs.get_branch().empty());

    REQUIRE( vcs.name() == "Auto");
    REQUIRE(!vcs.entry().get_command().is_open());

    wex::config(_("vcs.Base folder")).set(
      std::list<std::string>{wxGetCwd().ToStdString()});

    REQUIRE( vcs.set_entry_from_base());
    
    REQUIRE( vcs.use());
#endif
  }
}

TEST_SUITE_END();
