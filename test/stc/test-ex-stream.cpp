////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-stream.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/ex-stream.h>
#include <wex/stc.h>

TEST_CASE("wex::ex_stream")
{
  auto* stc = get_stc();

  SUBCASE("constructor")
  {
    wex::ex_stream exs(stc);
    REQUIRE(exs.get_current_line() == -1);
    REQUIRE(exs.get_line_count() == -1);

    exs.goto_line(5);
    REQUIRE(exs.get_current_line() == -1);
    REQUIRE(exs.get_line_count() == -1);
  }

  SUBCASE("stream")
  {
    std::fstream ifs("test.md");
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(stc);
    exs.stream(ifs);
    REQUIRE(stc->get_text() == "# markdown");
    REQUIRE(exs.get_current_line() == 0);
  }

  SUBCASE("find")
  {
    std::fstream ifs("test.md");
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(stc);
    exs.stream(ifs);

    REQUIRE(exs.find(std::string("one")));
    REQUIRE(exs.get_current_line() == 3);

    REQUIRE(!exs.find(std::string("xxxone")));
    REQUIRE(exs.get_current_line() == 3);
  }
}
