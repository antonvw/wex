////////////////////////////////////////////////////////////////////////////////
// Name:      test-hexmode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/hexmode.h>
#include <wex/lexers.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::hexmode")
{
  // 0000000000111111111122222222223333333333444444444455555555555666666666
  // 0123456789012345678901234567890123456789012345678901234567890123456789
  // 30 31 32 33 34 35 36 37 38 39                   0123456789
  wex::stc* stc = new wex::stc(
    std::string("0123456789"), wex::stc_data().flags(
      wex::stc_data::window_t().set(wex::stc_data::WIN_HEX)));

  wex::test::add_pane(frame(), stc);
  REQUIRE(stc->GetText() != "0123456789");
  
  stc->SetCurrentPos(48); // 0 <-
  
  wex::hexmode* hm = &stc->get_hexmode();
  
  REQUIRE( hm->is_active());
  REQUIRE( hm->get_stc() == stc);
  REQUIRE( hm->buffer() == "0123456789");
    
  hm->append_text("0123456789");
  REQUIRE( hm->buffer() == "01234567890123456789");
  REQUIRE( hm->highlight_other()); // current pos
  REQUIRE( hm->highlight_other(0));
  REQUIRE( hm->highlight_other(10));
  REQUIRE( hm->highlight_other(57));
  hm->undo();
  REQUIRE( hm->buffer() == "01234567890123456789");
  
  stc->DiscardEdits();  
  hm->set(false);
  REQUIRE( stc->GetText() == "01234567890123456789");
  
  // Test hex field.
  hm->set(true);
  REQUIRE(!hm->get_info().empty()); // 34 <- (ascii 4)
  REQUIRE(!hm->replace('x', 13));
  REQUIRE(!hm->replace('y', 13));
  REQUIRE(!hm->replace('g', 13));
  REQUIRE( hm->replace('a', 13));
  REQUIRE( hm->replace('9', 13));
  REQUIRE( hm->replace('2', 13));
  
  REQUIRE( stc->get_file().file_save(wex::test::get_path("test.hex")));
  hm->set(false);
  REQUIRE( stc->GetText() == "01232567890123456789");
  
  // Test ascii field.
  hm->set(true);
  REQUIRE(!hm->get_info().empty());
  REQUIRE( hm->replace('x', 54)); // 6 <-
  
  REQUIRE( stc->get_file().file_save());
  hm->set(false);
  REQUIRE(stc->GetText() == "012325x7890123456789");
  
  REQUIRE(!hm->is_active());
  
  hm->append_text("0123456789");
  REQUIRE(!hm->buffer().empty());

  hm->set(false);
  REQUIRE(!hm->is_active());

  // Test delete, insert.
  stc->set_text("0123456789");
  hm->set(true);
  REQUIRE( hm->is_active());
  REQUIRE( hm->buffer() == "0123456789");
  REQUIRE( hm->erase(1, 13));
  REQUIRE( hm->buffer() == "012356789");
  REQUIRE( hm->insert("30", 13)); // insert in hex field 
  REQUIRE( hm->buffer() == "0123056789");
  REQUIRE( hm->insert("abc", 52)); // insert in ascii field 
  REQUIRE( hm->buffer() == "0123abc056789");
  
  // Test replace target (replace in hex field).
  hm->set_text("0123456789");
  hm->set(true);
  stc->SetTargetStart(wxSTC_INVALID_POSITION);
  REQUIRE(!hm->replace_target("AA"));
  stc->SetTargetStart(3);
  stc->SetTargetEnd(4);
  REQUIRE( hm->replace_target("AA"));
  stc->SetTargetStart(3); // second byte
  stc->SetTargetEnd(6);
  REQUIRE(!hm->replace_target("A"));
  REQUIRE(!hm->replace_target("AA AB"));
  REQUIRE(!hm->replace_target("FG"));
  REQUIRE(!hm->replace_target("aAAB"));
  REQUIRE( hm->replace_target("AAAB"));
  stc->SetTargetStart(3);
  stc->SetTargetEnd(6);
  REQUIRE( hm->replace_target("2021"));
  REQUIRE( hm->buffer() == "0 !3456789");
  
  // If we have:
  // 30 31 32 33 34 35
  // RT: 31 32 -> 39
  //     30 39 33 34 35 (delete)
  // RT: 31 32 -> 39 39
  //     30 39 39 33 34 35 (replace)
  // RT: 31 32 -> 39 39 39
  //     30 39 39 39 33 34 35 (insert)
  hm->set_text("0123456789");
  REQUIRE( hm->buffer() == "0123456789");
  stc->SetTargetStart(3);
  stc->SetTargetEnd(9);
  REQUIRE( hm->replace_target("39"));
  REQUIRE( hm->buffer() == "093456789");
  hm->set_text("0123456789");
  stc->SetTargetStart(3);
  stc->SetTargetEnd(9);
  REQUIRE( hm->replace_target("3939"));
  REQUIRE( hm->buffer() == "0993456789");
  hm->set_text("0123456789");
  stc->SetTargetStart(3);
  stc->SetTargetEnd(9);
  REQUIRE( hm->replace_target("393939"));
  REQUIRE( hm->buffer() == "0999456789");

  // Test set text.
  hm->set_text("hello world");
  REQUIRE( hm->buffer() == "hello world");
  REQUIRE( hm->get_stc()->GetText() != "hello world");
  
  wxKeyEvent event(wxEVT_KEY_DOWN);
  hm->set_pos(event);
  
  wex::lexers::get()->apply(stc);
  
  REQUIRE( remove("test.hex") == 0);
  
  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
}
