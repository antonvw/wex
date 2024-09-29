////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/factory/line-data.h>

TEST_CASE("wex::factory::stc")
{
  auto* stc = new wex::test::stc();
  stc->set_text("more text\notherline\nother line");

  SUBCASE("margin")
  {
    REQUIRE(!stc->margin_text_is_shown());
    stc->SetMarginWidth(0, 10);
    stc->SetMarginWidth(1, 20);
    REQUIRE(stc->GetMarginWidth(0) == 10);
    REQUIRE(stc->GetMarginWidth(1) == 20);
    stc->margin_text_show();
    REQUIRE(stc->margin_text_is_shown());

    stc->reset_margins();
    REQUIRE(!stc->margin_text_is_shown());
    REQUIRE(stc->GetMarginWidth(0) == 0);
    REQUIRE(stc->GetMarginWidth(1) == 0);

    stc->SetMarginWidth(0, 10);
    stc->SetMarginWidth(1, 20);
    stc->reset_margins(
      wex::factory::stc::margin_t().set(wex::factory::stc::MARGIN_FOLDING));
    REQUIRE(stc->GetMarginWidth(0) == 10);
    REQUIRE(stc->GetMarginWidth(1) == 0);

    stc->SetMarginWidth(3, 10);
    stc->MarginSetText(0, "54321 xxx");
    REQUIRE(stc->get_margin_text_click() == -1);
    REQUIRE(stc->margin_get_revision_id().empty()); // not yet clicked

    stc->set_margin_text_click(5);
    REQUIRE(stc->get_margin_text_click() == 5);
    stc->set_margin_text_click(-1);
    REQUIRE(stc->get_margin_text_click() == -1);
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
    stc->bind_wx();

    REQUIRE(stc->get_selected_text().empty());

    stc->set_text("baan");
    stc->SelectAll();
    REQUIRE(stc->get_text() == "baan");
    REQUIRE(stc->get_selected_text() == "baan");

    REQUIRE(stc->vcs_renamed().empty());

    REQUIRE(!stc->eol().empty());
    REQUIRE(stc->get_fold_level() == 0);
    REQUIRE(stc->lexer_name().empty());
    REQUIRE(!stc->lexer_is_previewable());

    REQUIRE(stc->get_line_count() == 1);
    REQUIRE(stc->get_line_count_request() == 1);
  }

  SUBCASE("position")
  {
    stc->position_restore();
    stc->position_save();
    REQUIRE(stc->position_restore());
  }

  SUBCASE("text")
  {
    stc->clear();
    stc->add_text("baan");
    REQUIRE(stc->get_text() == "baan");

    stc->append_text("baan");
    REQUIRE(stc->get_text() == "baanbaan");
  }

  SUBCASE("virtual")
  {
    REQUIRE(!stc->vi_command(wex::line_data()));

    REQUIRE(!stc->vi_command_finish(true));

    REQUIRE(!stc->vi_is_recording());

    REQUIRE(!stc->vi_is_visual());

    REQUIRE(stc->vi_mode().empty());

    stc->generic_settings();
    stc->vi_record("xxx");
  }
}
