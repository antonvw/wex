////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcdlg.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/stcdlg.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExSTCEntryDialog", "[stc]")
{
  wxExSTCEntryDialog* dlg1 = new wxExSTCEntryDialog(GetFrame(), "hello", "testing");
  
  REQUIRE( dlg1->GetText() == "testing");
  //REQUIRE( dlg1.GetTextRaw() == "testing");
  REQUIRE(!dlg1->SetLexer("xxx"));
  REQUIRE( dlg1->SetLexer("cpp"));
  
  dlg1->Show();
  
  wxExSTCEntryDialog dlg2(
    GetFrame(), 
      "hello", 
      "testing",
      "hello again",
      wxOK);

  REQUIRE(!dlg2.GetText().empty());
  REQUIRE( dlg2.GetTextRaw().length() > 0);
}
