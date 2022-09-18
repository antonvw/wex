////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

TEST_CASE("wex::factory::stc")
{
  auto* stc = new wex::test::stc();
  stc->set_text("more text\notherline\nother line");

  SUBCASE("margin")
  {
    stc->SetMarginWidth(0, 10);
    REQUIRE(stc->GetMarginWidth(0) == 10);

    stc->reset_margins();
    REQUIRE(stc->GetMarginWidth(0) == 0);
  }

  SUBCASE("motion")
  {
    stc->BigWordLeft();
    stc->BigWordLeftExtend();
    stc->BigWordLeftRectExtend();
    stc->BigWordRight();
    stc->BigWordRightEnd();
    stc->BigWordRightEndExtend();
    stc->BigWordRightEndRectExtend();
    stc->BigWordRightExtend();
    stc->BigWordRightRectExtend();
    stc->LineHome();
    stc->LineHomeExtend();
    stc->LineHomeRectExtend();
    stc->LineScrollDownExtend();
    stc->LineScrollDownRectExtend();
    stc->LineScrollUpExtend();
    stc->LineScrollUpRectExtend();
    stc->PageScrollDown();
    stc->PageScrollDownExtend();
    stc->PageScrollDownRectExtend();
    stc->PageScrollUp();
    stc->PageScrollUpExtend();
    stc->PageScrollUpRectExtend();
    stc->ParaUpRectExtend();
    stc->ParaDownRectExtend();
    stc->WordLeftRectExtend();
    stc->WordRightRectExtend();
    stc->WordRightEndRectExtend();
  }

  SUBCASE("other")
  {
    REQUIRE(!stc->eol().empty());
    REQUIRE(stc->get_fold_level() == 0);
  }
}
