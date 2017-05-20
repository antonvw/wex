////////////////////////////////////////////////////////////////////////////////
// Name:      test-dialog.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/dialog.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExDialog")
{
  wxExDialog().Show();
  
  wxExDialog* dlg = new wxExDialog(wxExWindowData().Button(0).Title("no buttons"));
  dlg->Show();

  REQUIRE(dlg->GetData().Button() == 0);
  REQUIRE(dlg->GetData().Title() == "no buttons");
}
