////////////////////////////////////////////////////////////////////////////////
// Name:      test-client.cpp
// Purpose:   Unit tests for LSP diagnostics storage
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/client.h>
#include <wex/test/test.h>

TEST_CASE("wex::lsp::client")
{
  wex::lsp::client client("xx", "cpp");

  SECTION("initialize")
  {
    REQUIRE(client.initialize());
    REQUIRE(client.is_running());
    REQUIRE(client.is_initialized());
  }
}
