////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc_file.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/managed-frame.h>
#include <wex/stc-file.h>
#include <wex/stc.h>

TEST_CASE("wex::stc_file")
{
  auto* stc = new wex::stc(wex::test::get_path("test.h"));
  stc->set_text("and still they came");

  wex::test::add_pane(frame(), stc);

  wex::stc_file file(stc);

  // The file itself is not assigned.
  REQUIRE(!file.get_filename().stat().is_ok());
  REQUIRE(!file.get_contents_changed());

  REQUIRE(file.file_new("test-file.txt"));
  REQUIRE(stc->get_text().empty());
  stc->set_text("No, the game never ends "
                "when your whole world depends "
                "on the turn of a friendly card.");
  REQUIRE(!file.get_contents_changed());
  REQUIRE(file.file_save());
  REQUIRE(!file.get_contents_changed());
  REQUIRE(remove("test-file.txt") == 0);
}
