////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcdlg.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/stcdlg.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExSTCEntryDialog")
{
  wxExSTCEntryDialog* dlg1 = new wxExSTCEntryDialog("hello1", "testing1");
  
  REQUIRE( dlg1->GetSTC()->GetText() == "hello1");
  REQUIRE(!dlg1->GetSTC()->GetLexer().Set("xxx"));
  REQUIRE( dlg1->GetSTC()->GetLexer().Set("cpp"));
  
  dlg1->Show();
  
  wxExSTCEntryDialog* dlg2 = new wxExSTCEntryDialog("hello2", "testing2",
    wxExWindowData().Button(wxOK));
  
  REQUIRE(!dlg2->GetSTC()->GetText().empty());
  REQUIRE( dlg2->GetSTC()->GetTextRaw().length() > 0);
  
  dlg2->Show();
}
