////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcdlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/stcdlg.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::stc_entry_dialog")
{
  wex::stc_entry_dialog* dlg1 = new wex::stc_entry_dialog("hello1", "testing1");
  
  REQUIRE( dlg1->stc()->GetText() == "hello1");
  REQUIRE(!dlg1->stc()->get_lexer().set("xxx"));
  REQUIRE( dlg1->stc()->get_lexer().set("cpp"));
  
  dlg1->Show();
  
  wex::stc_entry_dialog* dlg2 = new wex::stc_entry_dialog("hello2", "testing2",
    wex::window_data().button(wxOK));
  
  REQUIRE(!dlg2->stc()->GetText().empty());
  REQUIRE( dlg2->stc()->GetTextRaw().length() > 0);
  
  dlg2->Show();
}
