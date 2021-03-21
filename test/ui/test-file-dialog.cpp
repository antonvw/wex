////////////////////////////////////////////////////////////////////////////////
// Name:      test-filedlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../test.h"
#include <wex/file-dialog.h>
#include <wex/file.h>
#include <wex/frame.h>

TEST_CASE("wex::file_dialog")
{
  SUBCASE("Default constructor")
  {
    wex::file_dialog dlg;
    REQUIRE(dlg.show_modal_if_changed(false) == wxID_CANCEL);
    REQUIRE(!dlg.is_hexmode());
  }

  SUBCASE("Other constructor")
  {
    wex::file        file;
    wex::file_dialog dlg(&file);
    REQUIRE(dlg.show_modal_if_changed(false) == wxID_OK);
    REQUIRE(!dlg.is_hexmode());
  }
}
