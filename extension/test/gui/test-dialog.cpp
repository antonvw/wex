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
#include <wex/dialog.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::dialog")
{
  wex::dialog().Show();
  
  wex::dialog* dlg = new wex::dialog(wex::window_data().button(0).title("no buttons"));
  dlg->Show();

  REQUIRE(dlg->data().button() == 0);
  REQUIRE(dlg->data().title() == "no buttons");
}
