////////////////////////////////////////////////////////////////////////////////
// Name:      test-tokenize.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/tokenize.h>

#include "../test.h"

import<list>;
import<vector>;

TEST_CASE("wex::tokenizer")
{
  REQUIRE(wex::tokenize<std::list<std::string>>(std::string()).size() == 0);
  REQUIRE(
    wex::tokenize<std::vector<std::string>>(std::string("x y z")).size() == 3);
}
