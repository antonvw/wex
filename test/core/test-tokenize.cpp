////////////////////////////////////////////////////////////////////////////////
// Name:      test-tokenize.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/tokenize.h>
#include <wex/test/test.h>

#include <list>
#include <vector>

TEST_CASE("wex::tokenizer")
{
  REQUIRE(wex::tokenize<std::list<std::string>>(std::string()).size() == 0);
  REQUIRE(
    wex::tokenize<std::vector<std::string>>(std::string("x y z")).size() == 3);
}
