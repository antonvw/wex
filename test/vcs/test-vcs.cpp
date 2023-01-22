////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log-none.h>
#include <wex/ui/menu.h>
#include <wex/vcs/vcs.h>

#include "test.h"

#include <vector>

TEST_CASE("wex::vcs")
{
  wex::path file(wex::test::get_path("test.h"));
  file.make_absolute();

  SUBCASE("statics")
  {
    REQUIRE(wex::vcs::load_document());
    REQUIRE(wex::vcs::dir_exists(file));
    REQUIRE(!wex::vcs::empty());
    REQUIRE(wex::vcs::size() > 0);
  }

  SUBCASE("others")
  {
    REQUIRE(wex::vcs::load_document());

    // using this constructor results in command id 3, being add.
    wex::vcs vcs(std::vector<wex::path>{file}, 3);

    REQUIRE(vcs.config_dialog(wex::data::window().button(wxAPPLY | wxCANCEL)));

#ifndef __WXMSW__
    REQUIRE(vcs.execute());
    REQUIRE(vcs.execute("status"));

    wex::log_none off;
    REQUIRE(!vcs.execute("xxx"));

    REQUIRE(vcs.show_dialog(wex::data::window().button(wxAPPLY | wxCANCEL)));

    REQUIRE(vcs.request(wex::data::window().button(wxAPPLY | wxCANCEL)));

    REQUIRE(vcs.entry().build_menu(100, new wex::menu("test", 0)) > 0);
    REQUIRE(vcs.entry().get_command().get_command() == "add");

    REQUIRE(!vcs.get_branch().empty());
    REQUIRE(vcs.toplevel().string().contains("wex"));

    REQUIRE(vcs.name() == "Auto");
    REQUIRE(!vcs.entry().get_command().is_open());

    wex::config(_("vcs.Base folder"))
      .set(std::list<std::string>{wxGetCwd().ToStdString()});

    REQUIRE(vcs.set_entry_from_base());

    REQUIRE(vcs.use());
#endif
  }
}
