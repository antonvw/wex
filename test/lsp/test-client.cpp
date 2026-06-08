////////////////////////////////////////////////////////////////////////////////
// Name:      test-client.cpp
// Purpose:   Unit tests for LSP client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/client.h>
#include <wex/syntax/lexers.h>
#include <wex/test/test.h>

TEST_CASE("wex::lsp::client")
{
  wex::lexers::get()->load_document();
  wex::lsp::client client(wex::lexer("cpp"));

  SECTION("initialize")
  {
    REQUIRE(client.language_id() == "cpp");
    REQUIRE(!client.is_running());
    REQUIRE(!client.is_initialized());

    REQUIRE(client.initialize(wex::test::get_path().string()));
    REQUIRE(client.language_id() == "cpp");
    REQUIRE(client.is_running());
    REQUIRE(client.is_initialized());
  }

  SECTION("others")
  {
    const std::string uri("file:///Users/anton/wex/test/data/test.h");

    REQUIRE(client.initialize(wex::test::get_path().string()));
    REQUIRE(client.hover("", 5, 5).empty());
    REQUIRE(client.hover(uri, 5, 5).empty());

    REQUIRE(client.did_open(uri, "cpp", "main() {}"));
    REQUIRE(!client.hover(uri, 1, 1).empty());
    REQUIRE(client.did_change(uri, "main() {xxx};"));
    REQUIRE(client.did_close(uri));
  }

  SECTION("shutdown")
  {
    REQUIRE(client.initialize(wex::test::get_path().string()));
    REQUIRE(client.shutdown());
    REQUIRE(!client.shutdown());
  }
}
