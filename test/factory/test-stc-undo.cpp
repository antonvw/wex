////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc-undo.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/stc-undo.h>

#include "test.h"

TEST_CASE("wex::stc_undo")
{
  auto* stc = new wex::test::stc();

  stc->set_text("aaaaa\nbbbbb\nccccc\nddddddddd\n");

  SUBCASE("action")
  {
    REQUIRE(stc->CanUndo());

    {
      wex::stc_undo undo(
        stc,
        wex::stc_undo::undo_t().set(wex::stc_undo::UNDO_ACTION));

      stc->AppendText("hello1");
      stc->AppendText("hello2");
      stc->AppendText("hello3");
    }

    stc->Undo();

    REQUIRE(!stc->get_text().contains("hello"));
  }

  SUBCASE("pos")
  {
    stc->SetCurrentPos(5);
    REQUIRE(stc->GetCurrentPos() == 5);

    {
      wex::stc_undo uno(
        stc,
        wex::stc_undo::undo_t().set(wex::stc_undo::UNDO_POS));
      stc->SetCurrentPos(25);
    }

    REQUIRE(stc->GetCurrentPos() == 5);
  }

  SUBCASE("sel-none")
  {
    stc->SelectNone();

    {
      wex::stc_undo undo(
        stc,
        wex::stc_undo::undo_t().set(wex::stc_undo::UNDO_SEL_NONE));
      stc->SetSelection(4, 8);
    }

    REQUIRE(stc->GetSelectedText().empty());
  }
}
