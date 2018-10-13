////////////////////////////////////////////////////////////////////////////////
// Name:      test-dialog.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/dialog.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wex::dialog")
{
  wex::dialog().Show();
  
  wex::dialog* dlg = new wex::dialog(wex::window_data().Button(0).Title("no buttons"));
  dlg->Show();

  REQUIRE(dlg->GetData().Button() == 0);
  REQUIRE(dlg->GetData().Title() == "no buttons");
}
