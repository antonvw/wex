////////////////////////////////////////////////////////////////////////////////
// Name:      test-dialog.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc-entry-dialog.h>
#include <wex/stc.h>

#include "../test.h"

TEST_CASE("wex::stc_entry_dialog")
{
  SUBCASE("constructor")
  {
    auto* dlg = new wex::stc_entry_dialog("hello", "testing");

    REQUIRE(dlg->get_stc()->get_text() == "hello");
    REQUIRE(!dlg->get_stc()->get_lexer().set("xxx"));
    REQUIRE(dlg->get_stc()->get_lexer().set("cpp"));

    dlg->Show();
  }

  SUBCASE("constructor-data")
  {
    wex::stc_entry_dialog* dlg = new wex::stc_entry_dialog(
      "hello",
      "testing",
      wex::data::window().button(wxOK));

    REQUIRE(!dlg->get_stc()->get_text().empty());

    dlg->Show();
  }

  SUBCASE("set_validator")
  {
    auto* dlg = new wex::stc_entry_dialog("hello", "testing");

    REQUIRE(!dlg->set_validator(""));
    REQUIRE(!dlg->set_validator("*"));
    REQUIRE(dlg->set_validator("[a-zA-Z]+[0-9]+"));
  }
}
