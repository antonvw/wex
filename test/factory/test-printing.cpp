////////////////////////////////////////////////////////////////////////////////
// Name:      test-printing.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/factory/printing.h>

TEST_CASE("wex::printing")
{
  REQUIRE(wex::printing::get() != nullptr);
  REQUIRE(wex::printing::get()->get_printer() != nullptr);
  REQUIRE(wex::printing::get()->get_html_printer() != nullptr);
  wex::printing* old = wex::printing::get();
  REQUIRE(wex::printing::get()->set(nullptr) == old);
  REQUIRE(wex::printing::get(false) == nullptr);
  REQUIRE(wex::printing::get(true) != nullptr);

  auto* frame    = new wxFrame(nullptr, wxID_ANY, "test");
  auto* stc      = new wxStyledTextCtrl(frame);
  auto* printout = new wex::printout(stc);

  printout->OnPreparePrinting();
  int min, max, from, to;
  printout->GetPageInfo(&min, &max, &from, &to);
  REQUIRE(!printout->HasPage(5));
  REQUIRE(!printout->OnPrintPage(5));

  SUBCASE("print_caption")
  {
    REQUIRE(
      wex::print_caption(wex::path("test")).find("test") != std::string::npos);
  }

  SUBCASE("print_footer")
  {
    REQUIRE(wex::print_footer().find("@") != std::string::npos);
  }

  SUBCASE("print_header")
  {
    REQUIRE(
      wex::print_header(wex::path_lexer(wex::test::get_path("test.h")))
        .find("test") != std::string::npos);
  }
}
