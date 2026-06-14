////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Unit tests for LSP client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/util.h>
#include <wex/test/test.h>

TEST_CASE("wex::lsp::util")
{
  SECTION("queue_event")
  {
    wex::lsp::queue_event(nullptr, "", 50, nullptr);
  }

  SECTION("range_from_json")
  {
    const std::string json_str("{\"range\":{\
         \"end\":{\"character\":30,\"line\":1153},\
         \"start\":{\"character\":19,\"line\":1151}}}");

    auto parsed = boost::json::parse(json_str);
    auto obj    = parsed.as_object();

    wex::range_item range;

    REQUIRE(range.start_line == 0);
    REQUIRE(range.start_character == 0);
    REQUIRE(range.end_line == 0);
    REQUIRE(range.end_character == 0);

    REQUIRE(wex::lsp::range_from_json(obj, range));

    REQUIRE(range.start_line == 1151);
    REQUIRE(range.start_character == 19);
    REQUIRE(range.end_line == 1153);
    REQUIRE(range.end_character == 30);
  }
}
