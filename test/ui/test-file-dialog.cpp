////////////////////////////////////////////////////////////////////////////////
// Name:      test-filedlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/file.h>
#include <wex/test/test.h>
#include <wex/ui/file-dialog.h>

TEST_CASE("wex::file_dialog")
{
  SECTION("constructor-no-file")
  {
    wex::file_dialog dlg(nullptr);

    REQUIRE(dlg.show_modal_if_changed(false) == wxID_CANCEL);
    REQUIRE(!dlg.is_hexmode());
  }

  SECTION("normal-constructor")
  {
    wex::file        file;
    wex::file_dialog dlg(&file);

    REQUIRE(dlg.show_modal_if_changed(false) == wxID_OK);
    REQUIRE(!dlg.is_hexmode());
  }
}
