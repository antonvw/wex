////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcdlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stcdlg.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::stc_entry_dialog")
{
  auto* dlg1 = new wex::stc_entry_dialog("hello1", "testing1");
  
  REQUIRE( dlg1->get_stc()->GetText() == "hello1");
  REQUIRE(!dlg1->get_stc()->get_lexer().set("xxx"));
  REQUIRE( dlg1->get_stc()->get_lexer().set("cpp"));
  
  dlg1->Show();
  
  wex::stc_entry_dialog* dlg2 = new wex::stc_entry_dialog("hello2", "testing2",
    wex::window_data().button(wxOK));
  
  REQUIRE(!dlg2->get_stc()->GetText().empty());
  REQUIRE( dlg2->get_stc()->GetTextRaw().length() > 0);
  
  dlg2->Show();
}
