////////////////////////////////////////////////////////////////////////////////
// Name:      test-evaluator.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log-none.h>
#include <wex/ex/ex.h>

#include "test.h"

#include "../src/ex/eval.h"

TEST_CASE("wex::evaluator")
{
  auto* stc = new wex::test::stc();
  stc->visual(true);
  auto* ex = new wex::ex(stc);
  stc->SetReadOnly(false);
  stc->set_text("xx\nxx\nyy\nzz\n");
  stc->DocumentStart();

  auto* eval = new wex::evaluator(ex);

  SECTION("eval")
  {
    REQUIRE(!eval->eval("").has_value());
  }

  SECTION("eval_token")
  {
    wex::log_none off;
    REQUIRE(eval->eval_token("") == 0);
    REQUIRE(eval->eval_token("xx") == 0);
    REQUIRE(eval->eval_token(".") == 1);
    REQUIRE(eval->eval_token("$") == 5);
    REQUIRE(eval->eval_token("+") == 1);
    REQUIRE(eval->eval_token("+", 10) == 11);
    REQUIRE(eval->eval_token("-", 10) == -9);
  }
}
