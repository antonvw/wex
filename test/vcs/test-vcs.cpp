////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log-none.h>
#include <wex/test/test.h>
#include <wex/ui/menu.h>
#include <wex/vcs/vcs.h>

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

  SUBCASE("constructor")
  {
    wex::vcs vcs(std::vector<wex::path>{file});
    REQUIRE(vcs.name() == "Auto");
    REQUIRE(vcs.entry().admin_dir() == ".git");

    vcs.set(wex::path("/tmp/xxx"));
    REQUIRE(vcs.name() == "Auto");
    REQUIRE(vcs.entry().admin_dir() != ".git"); // should be empty??
  }

  SUBCASE("usage")
  {
    wex::vcs vcs(std::vector<wex::path>{file});

    REQUIRE(vcs.use());

    const auto current(wex::config("vcs.VCS").get(0));

    REQUIRE(vcs.name() == "Auto");

    wex::config("vcs.VCS").set(0); // VCS_NONE

    REQUIRE(!vcs.use());
    vcs.entry() = wex::vcs_entry(); // should not be necessary
    CAPTURE(vcs.name());
    REQUIRE(vcs.name().empty());

    wex::config("vcs.VCS").set(current);

    REQUIRE(vcs.use());
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
      .set(wex::config::strings_t{wxGetCwd().ToStdString()});

    REQUIRE(vcs.set_entry_from_base());

    REQUIRE(vcs.use());

    auto tl(vcs.toplevel());

    REQUIRE(!tl.string().empty());

    REQUIRE(vcs.setup_exclude(vcs.toplevel()));

    REQUIRE(vcs.is_setup());

    REQUIRE(!vcs.is_dir_excluded(file.data().parent_path()));

    REQUIRE(vcs.is_dir_excluded(
      wex::path(tl).append(wex::path("external/wxWidgets"))));

    REQUIRE(!vcs.is_file_excluded(file));
#endif
  }
}
