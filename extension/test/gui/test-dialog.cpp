////////////////////////////////////////////////////////////////////////////////
// Name:      test-dialog.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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
  wxExDialog(GetFrame(), "hello").Show();
  
  wxExDialog* dlg = new wxExDialog(GetFrame(), "no buttons", 0);
  dlg->Show();
}
