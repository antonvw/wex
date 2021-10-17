////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc_file.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/file.h>

#include "test.h"

TEST_CASE("wex::stc_file")
{
  auto* stc = new wex::stc(wex::test::get_path("test.h"));
  stc->set_text("and still they came");

  frame()->pane_add(stc);

  wex::stc_file file(stc);

  // The file itself is not assigned.
  REQUIRE(!file.path().stat().is_ok());
  REQUIRE(!file.is_contents_changed());

  REQUIRE(file.file_new(wex::path("test-file.txt")));
  REQUIRE(stc->get_text().empty());
  stc->set_text("No, the game never ends "
                "when your whole world depends "
                "on the turn of a friendly card.");
  REQUIRE(!file.is_contents_changed());
  REQUIRE(file.file_save());
  REQUIRE(!file.is_contents_changed());
  REQUIRE(remove("test-file.txt") == 0);
}
