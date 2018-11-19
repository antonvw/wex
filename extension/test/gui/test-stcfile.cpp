////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc_file.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/stcfile.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::stc_file")
{
  wex::stc* stc = new wex::stc(GetTestPath("test.h"));
  stc->set_text("and still they came");
  
  AddPane(frame(), stc);
  
  wex::stc_file file(stc);

  // The file itself is not assigned.  
  REQUIRE(!file.get_filename().stat().is_ok());
  REQUIRE(!file.get_contents_changed());

  REQUIRE( file.file_new("test-file.txt"));
  REQUIRE( stc->GetText().empty());
  stc->set_text("No, the game never ends "
    "when your whole world depends "
    "on the turn of a friendly card.");
  REQUIRE(!file.get_contents_changed());
  REQUIRE( file.file_save());
  REQUIRE(!file.get_contents_changed());
  REQUIRE( remove("test-file.txt") == 0);
}
