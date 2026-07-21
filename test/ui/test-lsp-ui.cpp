////////////////////////////////////////////////////////////////////////////////
// Name:      test-lsp-ui.cpp
// Purpose:   Unit tests for LSP ui methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/ui/lsp.h>

#include "../src/ui/lsp-ui.h"
#include "test.h"

TEST_CASE("wex::lsp-ui")
{
  auto* stc = new wex::test::ui_stc();

  SECTION("make_path_skip_uri")
  {
    REQUIRE(wex::make_path_skip_uri(std::string()).empty());
    REQUIRE(wex::make_path_skip_uri("file:///path/to/file") == "/path/to/file");
    REQUIRE(wex::make_path_skip_uri("file:///path/to/file%20xyz") == "/path/to/file xyz");
  }

  SECTION("set_lsp_completions")
  {
    wex::completion_item_element item;
    item.label = "label";
    item.kind = 1;
    item.detail = "detail";
    item.documentation = "documentation";

    wex::set_lsp_completions(stc, &item, frame());
  } 

  SECTION("set_lsp_definition_or_implementation")
  {
    wex::definition_or_implementation_item item;
    item.uri = "file:///project/src/utils.ts";
    item.range.start.line = 10;
    item.range.start.character = 2;
    item.range.end.line = 11;
    item.range.end.character = 5;

    wex::set_lsp_definition_or_implementation(frame(), &item);
  }
  
  SECTION("set_lsp_diagnostics")
  {
    wex::diagnostic_item item;
    item.range.start.line = 10;
    item.range.start.character = 2;
    item.range.end.line = 11;
    item.range.end.character = 5;

    wex::set_lsp_diagnostics(stc, &item);
  }

  SECTION("set_lsp_hover")
  {
    wex::hover_item item;
    item.range.start.line = 10;
    item.range.start.character = 2;

    wex::set_lsp_hover(stc, &item);
  }

  SECTION("set_lsp_on_type")
  {
    wex::on_type_formatting_item item;
    item.range.start.line = 10;
    item.range.start.character = 2;

    wex::set_lsp_on_type(stc, &item);
  }

  SECTION("set_lsp_show_message")
  {
    wex::show_message_item item;
    item.type = wex::severity_t::INFO;
    item.message = "Test message";

    wex::set_lsp_show_message(frame(), &item);
  }
}
