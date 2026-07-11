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
    REQUIRE(!client.get_capabilities().support(
      wex::lsp::capabilities::CAP_COMPLETION));
    REQUIRE(!client.get_capabilities().support(255));
    REQUIRE(client.get_capabilities().trigger_completion_characters().empty());
    REQUIRE(client.language_id() == "cpp");
    REQUIRE(
      client.extensions() ==
      "*.cpp;*.c;*.cc;*.h;*.hpp;*.inl;*.cxx;*.l;*.y;*.sc");
    REQUIRE(!client.is_running());
    REQUIRE(!client.is_initialized());

    REQUIRE(client.initialize(wex::test::get_path()));
    REQUIRE(client.language_id() == "cpp");
    REQUIRE(client.is_running());
    REQUIRE(client.is_initialized());
    REQUIRE(!client.get_capabilities().support(
      wex::lsp::capabilities::CAP_DEFINITION));
    REQUIRE(client.get_capabilities().trigger_completion_characters().empty());
    REQUIRE(client.shutdown());
  }

  SECTION("others")
  {
    const wex::path path("/Users/anton/wex/test/data/test.h");

    REQUIRE(client.initialize(wex::test::get_path()));
    REQUIRE(!client.completion(wex::path(), wex::position_item(5, 5)));
    REQUIRE(client.definition(wex::path(), wex::position_item(5, 5)));
    REQUIRE(!client.hover(wex::path(), wex::position_item(5, 5)));
    REQUIRE(!client.hover(path, wex::position_item(5, 5)));
    REQUIRE(client.implementation(wex::path(), wex::position_item(5, 5)));

    REQUIRE(client.version(path.uri()) == 0);
    REQUIRE(client.did_open(path, "main() {}"));
    REQUIRE(client.version(path.uri()) == 1);
    REQUIRE(!client.hover(path, wex::position_item(1, 1)));
    REQUIRE(client.did_change(path, wex::range_item(), "main() {xxx};"));
    REQUIRE(client.version(path.uri()) == 2);
    REQUIRE(client.did_close(path));
    REQUIRE(client.version(path.uri()) == 2);
    REQUIRE(client.shutdown());
    REQUIRE(client.version(path.uri()) == 0);
  }

  SECTION("shutdown")
  {
    REQUIRE(client.initialize(wex::test::get_path()));
    REQUIRE(client.shutdown());
    REQUIRE(!client.shutdown());
  }
}
