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
  get_stc()->set_text("aaaaa\nbbbbb\nccccc\nddddddddd\n");

  SUBCASE("action")
  {
    REQUIRE(!get_stc()->CanUndo());

    {
      wex::stc_undo undo(
        get_stc(),
        wex::stc_undo::undo_t().set(wex::stc_undo::UNDO_ACTION));

      get_stc()->AppendText("hello1");
      get_stc()->AppendText("hello2");
      get_stc()->AppendText("hello3");
    }

    get_stc()->Undo();

    REQUIRE(get_stc()->get_text().find("hello") == std::string::npos);
  }

  SUBCASE("pos")
  {
    get_stc()->SetCurrentPos(5);

    {
      wex::stc_undo uno(
        get_stc(),
        wex::stc_undo::undo_t().set(wex::stc_undo::UNDO_POS));
      get_stc()->SetCurrentPos(25);
    }

    REQUIRE(get_stc()->GetCurrentPos() == 5);
  }

  SUBCASE("sel-none")
  {
    get_stc()->SelectNone();

    {
      wex::stc_undo undo(
        get_stc(),
        wex::stc_undo::undo_t().set(wex::stc_undo::UNDO_SEL_NONE));
      get_stc()->SetSelection(4, 8);
    }

    REQUIRE(get_stc()->GetSelectedText().empty());
  }
}
