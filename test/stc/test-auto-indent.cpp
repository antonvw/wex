////////////////////////////////////////////////////////////////////////////////
// Name:      test-auto-indent.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/factory/defs.h>
#include <wex/stc/auto-indent.h>

#include "test.h"

TEST_CASE("wex::auto_indent")
{
  auto* stc = get_stc();
  stc->get_vi().command("\x1b");

  wex::auto_indent ai(stc);

  SUBCASE("constructor")
  {
    REQUIRE(ai.use());
  }

  SUBCASE("level")
  {
    REQUIRE(stc->get_lexer().set("cpp", true));
    REQUIRE(stc->GetIndent() == 2);

    stc->set_text("\nif ()\n{\n\n");
    stc->DocumentEnd();
    ai.on_char(stc->eol().front()); // fold level == 0
  }

  SUBCASE("newline")
  {
    stc->set_text("  \n  line with indentation");
    stc->SetEOLMode(wxSTC_EOL_CR);
    stc->DocumentEnd();
    ai.on_char(stc->eol().front());
    // the \n is not added, but indentation does
    REQUIRE(stc->get_text() == "  \n  line with indentation");
    REQUIRE(stc->get_line_count() == 2);
  }

  SUBCASE("other")
  {
    stc->set_text("  \n  line with indentation");
    stc->DocumentEnd();
    REQUIRE(!ai.on_char('x'));
    REQUIRE(stc->get_text() == "  \n  line with indentation");
    REQUIRE(stc->get_line_count() == 2);
  }

  SUBCASE("use")
  {
    wex::config(_("stc.Auto indent")).set(true);
    REQUIRE(wex::auto_indent::use());

    wex::config(_("stc.Auto indent")).set(false);
    REQUIRE(!wex::auto_indent::use());

    wex::auto_indent::use(true);
    REQUIRE(wex::auto_indent::use());
  }
}
