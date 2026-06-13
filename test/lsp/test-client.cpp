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

    REQUIRE(client.initialize(wex::test::get_path()));
    REQUIRE(client.language_id() == "cpp");
    REQUIRE(client.is_running());
    REQUIRE(client.is_initialized());
  }

  SECTION("others")
  {
    const wex::path path("/Users/anton/wex/test/data/test.h");

    REQUIRE(client.initialize(wex::test::get_path()));
    REQUIRE(client.hover(wex::path(), wex::position_item(5, 5)));
    REQUIRE(client.hover(path, wex::position_item(5, 5)));

    REQUIRE(client.did_open(path, "main() {}"));
    REQUIRE(client.hover(path, wex::position_item(1, 1)));
    REQUIRE(client.did_change(path, "main() {xxx};"));
    REQUIRE(client.did_close(path));
  }

  SECTION("shutdown")
  {
    REQUIRE(client.initialize(wex::test::get_path()));
    REQUIRE(client.shutdown());
    REQUIRE(!client.shutdown());
  }
}
