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
#include <wx/extension/stcdlg.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wex::stc_entry_dialog")
{
  wex::stc_entry_dialog* dlg1 = new wex::stc_entry_dialog("hello1", "testing1");
  
  REQUIRE( dlg1->GetSTC()->GetText() == "hello1");
  REQUIRE(!dlg1->GetSTC()->GetLexer().Set("xxx"));
  REQUIRE( dlg1->GetSTC()->GetLexer().Set("cpp"));
  
  dlg1->Show();
  
  wex::stc_entry_dialog* dlg2 = new wex::stc_entry_dialog("hello2", "testing2",
    wex::window_data().Button(wxOK));
  
  REQUIRE(!dlg2->GetSTC()->GetText().empty());
  REQUIRE( dlg2->GetSTC()->GetTextRaw().length() > 0);
  
  dlg2->Show();
}
