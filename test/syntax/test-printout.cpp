////////////////////////////////////////////////////////////////////////////////
// Name:      test-printout.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/printout.h>
#include <wex/test/test.h>
#include <wx/stc/stc.h>

TEST_CASE("wex::printout")
{
  auto* frame    = new wxFrame(nullptr, wxID_ANY, "test");
  auto* stc      = new wxStyledTextCtrl(frame);
  auto* printout = new wex::printout(stc);

  printout->OnPreparePrinting();
  int min, max, from, to;
  printout->GetPageInfo(&min, &max, &from, &to);
  REQUIRE(!printout->HasPage(5));
  REQUIRE(!printout->OnPrintPage(5));
}
