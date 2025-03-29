////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log-none.h>
#include <wex/core/path.h>
#include <wex/syntax/blame.h>
#include <wex/syntax/indicator.h>
#include <wex/syntax/lexers.h>

#include "test.h"

TEST_CASE("wex::syntax::stc")
{
  auto* stc = new wex::test::stc();
  stc->set_text("more text\notherline\nother line");

  SECTION("fold")
  {
    wex::log_none off; // no indicator loaded
    REQUIRE(!stc->set_indicator(wex::indicator(4, 5), 100, 200));
  }

  SECTION("fold")
  {
    wex::config(_("stc.Auto fold")).set(3);
    stc->fold();
    stc->fold(true);
  }

  SECTION("lexers")
  {
    REQUIRE(stc->get_lexer().get_stc() == stc);
    REQUIRE(stc->get_lexer().display_lexer().empty());

    CAPTURE(stc->lexer_name());
    REQUIRE(stc->lexer_name().empty());
    REQUIRE(!stc->lexer_is_previewable());

    // it seems generic_settings is not invoked
    REQUIRE(stc->get_lexer().set("cpp"));
    REQUIRE(stc->get_lexer().get_stc() == stc);
    REQUIRE(stc->lexer_name() == "cpp");
    CHECK(stc->GetEdgeMode() == wxSTC_EDGE_NONE);

    wex::lexer lexer2(stc);
    REQUIRE(lexer2.get_stc() == stc);
    lexer2.set("markdown");
    REQUIRE(!lexer2.attribs().empty());
    REQUIRE(lexer2.attrib(_("Edge line")) == wxSTC_EDGE_NONE);
    REQUIRE(stc->GetEdgeMode() == wxSTC_EDGE_NONE);

    for (const auto& l : wex::lexers::get()->get_lexers())
    {
      if (!l.scintilla_lexer().empty())
      {
        CAPTURE(l.scintilla_lexer());
        wex::lexer one(l.scintilla_lexer());
        REQUIRE(one.is_ok());
#ifndef __WXMSW__
        wex::lexer two(stc);
        REQUIRE(two.set(one));
        REQUIRE(two.is_ok());
#endif
      }
    }

    REQUIRE(!wex::lexer().is_ok());
    REQUIRE(!wex::lexer(" cpp").is_ok());
    REQUIRE(!wex::lexer("cpp ").is_ok());
    REQUIRE(!wex::lexer("xxx").is_ok());
    stc->get_lexer().set("cpp");
    wex::lexers::get()->apply_global_styles(stc);
    wex::lexers::get()->apply(stc);
  }

  SECTION("margin")
  {
    wex::blame blame;
    stc->blame_margin(&blame);

    REQUIRE(stc->margin_get_revision_renamed().empty());
  }
}
