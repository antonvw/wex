////////////////////////////////////////////////////////////////////////////////
// Name:      test-filedlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/filedlg.h>
#include <wex/file.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::file_dialog")
{
  wex::file file;
  wex::file_dialog dlg(&file);
  
  REQUIRE(dlg.show_modal_if_changed(false) == wxID_OK);
}
