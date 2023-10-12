////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcsentry.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/core/path.h>
#include <wex/factory/defs.h>
#include <wex/syntax/lexers.h>
#include <wex/vcs/vcs-entry.h>

#include "test.h"

TEST_CASE("wex::vcs_entry")
{
  SUBCASE("default constructor")
  {
    REQUIRE(wex::vcs_entry().get_commands().empty());
    REQUIRE(
      wex::vcs_entry().flags_location() ==
      wex::vcs_entry::flags_location_t::POSTFIX);
  }

  SUBCASE("constructor using xml")
  {
    pugi::xml_document doc;

    REQUIRE(doc.load_string("<vcs name=\"git\" admin-dir=\"./\" log-flags=\"-n "
                            "1\" blame-format=\" yyyy\">"
                            "  <commands>"
                            "     <command> help </command>"
                            "     <command> blame </command>"
                            "  </commands>"
                            "</vcs>"));

    wex::vcs_entry entry(doc.document_element());
    REQUIRE(entry.name() == "git");

    wex::log_none off;
    REQUIRE(!entry.log(wex::test::get_path("test.h"), "x"));

    REQUIRE(entry.log(wex::test::get_path("test.h"), "-1"));
    REQUIRE(entry.get_blame().use());

    REQUIRE(entry.get_commands().size() == 2);
    REQUIRE(!entry.get_command().get_command().empty());
    REQUIRE(entry.admin_dir() == "./");
    REQUIRE(entry.get_flags().empty());

#ifndef __WXMSW__
    // the get_branch gives error on msw
    REQUIRE(!entry.get_branch().empty());
    REQUIRE(!entry.std_out().empty());
#endif
    entry.show_output();

    wex::menu menu;
    REQUIRE(entry.build_menu(5, &menu) == 2);

#ifndef __WXMSW__
    // This should have no effect.
    REQUIRE(!entry.set_command(5));
    REQUIRE(!entry.set_command(wex::ID_EDIT_VCS_LOWEST));

    REQUIRE(entry.get_commands().size() == 2);
    REQUIRE(entry.get_flags().empty());
    REQUIRE(!entry.std_out().empty());
    REQUIRE(entry.execute()); // executes just git, shows help
    REQUIRE(entry.std_out().contains("usage: "));
    entry.show_output();

    REQUIRE(entry.system(wex::process_data("help")) == 0);

    auto* other = new wex::vcs_entry(doc.document_element());
    REQUIRE(other->execute(std::string(), wex::lexer()));
    other->show_output();
#endif
  }

  SUBCASE("blame")
  {
    auto*      stc = get_stc();
    wex::blame blame;
    wex::lexers::get()->apply_margin_text_style(stc, &blame);
    auto* entry = load_git_entry();

#ifndef __WXMSW__
    REQUIRE(
      entry->system(wex::process_data().args(
        "blame " + wex::test::get_path("test.h").string())) == 0);
#endif

    stc->get_file().reset_contents_changed();
  }
}
