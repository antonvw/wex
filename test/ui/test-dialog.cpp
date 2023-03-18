////////////////////////////////////////////////////////////////////////////////
// Name:      test-dialog.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/ui/dialog.h>
#include <wex/ui/frame.h>

TEST_CASE("wex::dialog")
{
  wex::dialog().Show();

  auto* dlg =
    new wex::dialog(wex::data::window().button(0).title("no buttons"));
  dlg->Show();

  REQUIRE(dlg->data().button() == 0);
  REQUIRE(dlg->data().title() == "no buttons");
}
