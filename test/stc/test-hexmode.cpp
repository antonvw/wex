////////////////////////////////////////////////////////////////////////////////
// Name:      test-hexmode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/lexers.h>
#include <wex/stc/hexmode.h>

#include "test.h"

TEST_CASE("wex::hexmode")
{
  // 0000000000111111111122222222223333333333444444444455555555555666666666
  // 0123456789012345678901234567890123456789012345678901234567890123456789
  // 30 31 32 33 34 35 36 37 38 39                   0123456789
  auto* stc = new wex::stc(
    std::string("0123456789"),
    wex::data::stc().flags(
      wex::data::stc::window_t().set(wex::data::stc::WIN_HEX)));

  frame()->pane_add(stc);
  REQUIRE(stc->get_text() != "0123456789");

  stc->SetCurrentPos(48); // 0 <-

  auto* hm = &stc->get_hexmode();

  SUBCASE("initial")
  {
    REQUIRE(hm->is_active());
    REQUIRE(hm->get_stc() == stc);
    REQUIRE(hm->buffer() == "0123456789");
  }

  hm->set(true);

  SUBCASE("append")
  {
    hm->append_text("0123456789");
    REQUIRE(hm->buffer() == "01234567890123456789");
    REQUIRE(hm->highlight_other()); // current pos
    REQUIRE(hm->highlight_other(0));
    REQUIRE(hm->highlight_other(10));
    REQUIRE(hm->highlight_other(57));

    hm->undo();
    REQUIRE(hm->buffer() == "01234567890123456789");

    stc->DiscardEdits();
    hm->set(false);
    REQUIRE(stc->get_text() == "01234567890123456789");
  }

  SUBCASE("hex field")
  {
    REQUIRE(!hm->get_info().empty()); // 34 <- (ascii 4)
    REQUIRE(!hm->replace('x', 13));
    REQUIRE(!hm->replace('y', 13));
    REQUIRE(!hm->replace('g', 13));
    REQUIRE(hm->replace('a', 13));
    REQUIRE(hm->replace('9', 13));
    REQUIRE(hm->replace('2', 13));

    REQUIRE(stc->get_file().file_save(wex::test::get_path("test.hex")));
    hm->set(false);
    REQUIRE(stc->get_text() == "0123256789");
    REQUIRE(stc->get_file().file_save());
  }

  SUBCASE("ascii field")
  {
    REQUIRE(!hm->get_info().empty());
    REQUIRE(hm->replace('x', 54)); // 6 <-

    hm->set(false);
    REQUIRE(stc->get_text() == "012345x789");
    REQUIRE(!hm->is_active());

    hm->append_text("0123456789");
    REQUIRE(!hm->buffer().empty());

    hm->set(false);
    REQUIRE(!hm->is_active());
  }

  SUBCASE("erase insert")
  {
    REQUIRE(hm->is_active());
    REQUIRE(hm->buffer() == "0123456789");
    REQUIRE(hm->erase(1, 13));
    REQUIRE(hm->buffer() == "012356789");
    REQUIRE(hm->insert("30", 13)); // insert in hex field
    REQUIRE(hm->buffer() == "0123056789");
    REQUIRE(hm->insert("abc", 52)); // insert in ascii field
    REQUIRE(hm->buffer() == "0123abc056789");
  }

  SUBCASE("replace target (replace in hex field)")
  {
    stc->SetTargetStart(wxSTC_INVALID_POSITION);
    REQUIRE(!hm->replace_target("AA"));

    stc->SetTargetStart(3);
    stc->SetTargetEnd(4);
    REQUIRE(hm->replace_target("AA"));

    stc->SetTargetStart(3); // second byte
    stc->SetTargetEnd(6);
    REQUIRE(!hm->replace_target("A"));
    REQUIRE(!hm->replace_target("AA AB"));
    REQUIRE(!hm->replace_target("FG"));
    REQUIRE(!hm->replace_target("aAAB"));
    REQUIRE(hm->replace_target("AAAB"));

    stc->SetTargetStart(3);
    stc->SetTargetEnd(6);
    REQUIRE(hm->replace_target("2021"));
    REQUIRE(hm->buffer() == "0 !3456789");

    // If we have:
    // 30 31 32 33 34 35
    // RT: 31 32 -> 39
    //     30 39 33 34 35 (delete)
    // RT: 31 32 -> 39 39
    //     30 39 39 33 34 35 (replace)
    // RT: 31 32 -> 39 39 39
    //     30 39 39 39 33 34 35 (insert)
    hm->set_text("0123456789");
    REQUIRE(hm->buffer() == "0123456789");

    stc->SetTargetStart(3);
    stc->SetTargetEnd(9);
    REQUIRE(hm->replace_target("39"));
    REQUIRE(hm->buffer() == "093456789");

    hm->set_text("0123456789");
    stc->SetTargetStart(3);
    stc->SetTargetEnd(9);
    REQUIRE(hm->replace_target("3939"));
    REQUIRE(hm->buffer() == "0993456789");

    hm->set_text("0123456789");
    stc->SetTargetStart(3);
    stc->SetTargetEnd(9);
    REQUIRE(hm->replace_target("393939"));
    REQUIRE(hm->buffer() == "0999456789");
  }

  SUBCASE("set text")
  {
    hm->set_text("hello world");
    REQUIRE(hm->buffer() == "hello world");
    REQUIRE(hm->get_stc()->get_text() != "hello world");

    wxKeyEvent event(wxEVT_KEY_DOWN);
    hm->set_pos(event);

    wex::lexers::get()->apply(stc);
  }

  SUBCASE("finish")
  {
    REQUIRE(remove("test.hex") == 0);

    stc->EmptyUndoBuffer();
    stc->SetSavePoint();
  }
}
