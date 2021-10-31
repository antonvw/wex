////////////////////////////////////////////////////////////////////////////////
// Name:      test-filedlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/file.h>
#include <wex/ui/file-dialog.h>
#include <wex/ui/frame.h>

TEST_CASE("wex::file_dialog")
{
  SUBCASE("default-constructor")
  {
    wex::file_dialog dlg;
    REQUIRE(dlg.show_modal_if_changed(false) == wxID_CANCEL);
    REQUIRE(!dlg.is_hexmode());
  }

  SUBCASE("other-constructor")
  {
    wex::file        file;
    wex::file_dialog dlg(&file);
    REQUIRE(dlg.show_modal_if_changed(false) == wxID_OK);
    REQUIRE(!dlg.is_hexmode());
  }
}
