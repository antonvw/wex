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
}
