////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcdlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/stc.h>
#include <wex/stcdlg.h>

TEST_CASE("wex::stc_entry_dialog")
{
  auto* dlg1 = new wex::stc_entry_dialog("hello1", "testing1");

  REQUIRE(dlg1->get_stc()->get_text() == "hello1");
  REQUIRE(!dlg1->get_stc()->get_lexer().set("xxx"));
  REQUIRE(dlg1->get_stc()->get_lexer().set("cpp"));

  dlg1->Show();

  wex::stc_entry_dialog* dlg2 = new wex::stc_entry_dialog(
    "hello2",
    "testing2",
    wex::window_data().button(wxOK));

  REQUIRE(!dlg2->get_stc()->get_text().empty());

  dlg2->Show();
}
