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
    wex::completions_t items;

    wex::completion_item_element item;
    item.label = "label";
    item.kind = 1;
    item.detail = "detail";
    item.documentation = "documentation";

    items.push_back(item);

    wex::set_lsp_completions(stc, &items, frame());
  } 

  SECTION("set_lsp_definition_or_implementation")
  {
    wex::definition_or_implementation_t items;

    wex::definition_or_implementation_item item(
      "file:///project/src/utils.ts",
      wex::range_item(wex::position_item(10, 2), wex::position_item(11, 5)));
    items.push_back(item);

    wex::set_lsp_definition_or_implementation(frame(), &items);
  }
  
  SECTION("set_lsp_diagnostics")
  {
    wex::diagnostics_t diagnostics;

    diagnostics.push_back(wex::diagnostic_item(
      wex::range_item(wex::position_item(10, 2), wex::position_item(11, 5)),
      "Test diagnostic message"));

    wex::set_lsp_diagnostics(stc, &diagnostics);
  }

  SECTION("set_lsp_hover")
  {
    wex::hover_item item(wex::position_item(10, 2), "Test hover contents");

    wex::set_lsp_hover(stc, &item);
  }

  SECTION("set_lsp_on_type")
  {
    wex::on_type_formatting_item_t items;
    items.push_back(wex::on_type_formatting_item(
      wex::range_item(wex::position_item(12, 2), wex::position_item(12, 5)), "xyz");
    items.push_back(wex::on_type_formatting_item(
      wex::range_item(wex::position_item(10, 2), wex::position_item(11, 5)), "abc"));

    wex::set_lsp_on_type(stc, &items);
  }

  SECTION("set_lsp_show_message")
  {
    wex::show_message_item item(
      "Test message",
      wex::severity_t::INFO);

    wex::set_lsp_show_message(frame(), &item);
  }
}
