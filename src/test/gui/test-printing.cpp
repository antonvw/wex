////////////////////////////////////////////////////////////////////////////////
// Name:      test-printing.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/printing.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::printing")
{
  REQUIRE(wex::printing::get() != nullptr);
  REQUIRE(wex::printing::get()->get_printer() != nullptr);
  REQUIRE(wex::printing::get()->get_html_printer() != nullptr);
  wex::printing* old = wex::printing::get();
  REQUIRE(wex::printing::get()->set(nullptr) == old);
  REQUIRE(wex::printing::get(false) == nullptr);
  REQUIRE(wex::printing::get(true) != nullptr);
  
  auto* printout = new wex::printout(get_stc());
  
  printout->OnPreparePrinting();
  int min, max, from, to;
  printout->GetPageInfo(&min, &max, &from, &to);
  REQUIRE(!printout->HasPage(5));
  REQUIRE(!printout->OnPrintPage(5));

  // these wait for a click
//  stc->Print(false);
//  stc->print_preview(wxPreviewFrame_NonModal);
}
