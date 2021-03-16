////////////////////////////////////////////////////////////////////////////////
// Name:      test-dialog.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/dialog.h>
#include <wex/frame.h>
#include "../test.h"

TEST_CASE("wex::dialog")
{
  wex::dialog().Show();
  
  auto* dlg = new wex::dialog(wex::data::window().button(0).title("no buttons"));
  dlg->Show();

  REQUIRE(dlg->data().button() == 0);
  REQUIRE(dlg->data().title() == "no buttons");
}
