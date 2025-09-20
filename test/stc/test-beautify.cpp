////////////////////////////////////////////////////////////////////////////////
// Name:      test-beautify.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/stc/beautify.h>

#include "test.h"

TEST_CASE("wex::beautify")
{
  SECTION("access")
  {
    REQUIRE(!wex::beautify().is_supported(wex::lexer("pascal")));
    REQUIRE(wex::beautify().is_supported(wex::lexer("cpp")));
  }

  wex::config("stc.Beautifier").set(wex::config::strings_t{{"clang-format"}});

#ifndef __WXMSW__
  SECTION("stc")
  {
    auto* stc = get_stc();

    REQUIRE(stc->get_lexer().set("cpp", true));
    stc->set_text("if (x) {}\n");

    wex::log_none off;

    if (wex::beautify().stc(*stc))
    {
      REQUIRE(stc->get_text() == "if (x)\n{\n}\n");
    }
    else
    {
      REQUIRE(stc->get_text() == "if (x) {}\n");
    }
  }
#endif

  wex::config("stc.Beautifier").set(wex::config::strings_t{{""}});
}
