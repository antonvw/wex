////////////////////////////////////////////////////////////////////////////////
// Name:      test-commandline-imp.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/ex-commandline-input.h>
#include <wex/ui/ex-commandline.h>

#include "../src/ui/ex-commandline-imp.h"
#include "test.h"

TEST_CASE("wex::ex_commandline_imp")
{
  auto* stc     = get_stc();
  auto* control = new wxControl(frame(), 100);
  auto* cl = new wex::ex_commandline(frame(), control, wex::data::window());

  frame()->pane_add(control);

  stc->set_text("hello\nhello11\nhello22\ntest\ngcc\nblame\nthis\nyank\ncopy");

  SUBCASE("constructor")
  {
    wex::ex_commandline_imp cli(cl, control, wex::data::window());

    REQUIRE(cli.text_not_expanded().empty());
    REQUIRE(cli.handle("/hello"));
    REQUIRE(cli.handle('c'));
  }
}
