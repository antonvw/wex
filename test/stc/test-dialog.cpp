////////////////////////////////////////////////////////////////////////////////
// Name:      test-dialog.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/stc/entry-dialog.h>
#include <wex/stc/stc.h>

#include <wex/test/test.h>

TEST_CASE("wex::stc_entry_dialog")
{
  SUBCASE("constructor")
  {
    auto* dlg = new wex::stc_entry_dialog("hello", "testing");

    REQUIRE(dlg->get_stc()->get_text() == "hello");
    REQUIRE(!dlg->get_stc()->get_lexer().set("xxx"));
    REQUIRE(dlg->get_stc()->get_lexer().set("cpp"));
    REQUIRE(dlg->data().style() == wex::data::NUMBER_NOT_SET);
    REQUIRE(
      !dlg->get_stc()->data().flags().test(wex::data::stc::WIN_SINGLE_LINE));

    dlg->Show();
  }

  SUBCASE("constructor-data")
  {
    wex::stc_entry_dialog* dlg = new wex::stc_entry_dialog(
      "hello",
      "testing",
      wex::data::window().button(wxOK).style(0),
      wex::data::stc().flags(
        wex::data::stc::window_t().set(wex::data::stc::WIN_SINGLE_LINE)));

    REQUIRE(!dlg->get_stc()->get_text().empty());
    REQUIRE(
      dlg->get_stc()->data().flags().test(wex::data::stc::WIN_SINGLE_LINE));

    dlg->Show();
  }

  SUBCASE("set_validator")
  {
    auto* dlg = new wex::stc_entry_dialog("hello", "testing");

    REQUIRE(!dlg->set_validator(""));
    wex::log_none off;
    REQUIRE(!dlg->set_validator("*"));
    REQUIRE(dlg->set_validator("[a-zA-Z]+[0-9]+"));
  }
}
