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
    REQUIRE(
      wex::make_path_skip_uri("file:///path/to/file").string() ==
      "/path/to/file");
    REQUIRE(
      wex::make_path_skip_uri("file:///path/to/file%20xyz").string() ==
      "/path/to/file xyz");
  }

  SECTION("set_lsp_completions")
  {
    auto* items = new wex::completions_t;

    wex::completion_item_element item("label");
    items->elements.emplace_back(item);

    wex::set_lsp_completions(stc, items, frame());
  }

  SECTION("set_lsp_definition_or_implementation")
  {
    auto* items = new wex::definition_or_implementation_t;

    wex::definition_or_implementation_item item(
      "file:///project/src/utils.ts",
      wex::range_item(wex::position_item(10, 2), wex::position_item(11, 5)));
    items->emplace_back(item);

    wex::set_lsp_definition_or_implementation(frame(), items);
  }

  SECTION("set_lsp_diagnostics")
  {
    auto* diagnostics = new wex::diagnostics_t;

    diagnostics->emplace_back(
      wex::diagnostic_item(
        wex::range_item(wex::position_item(10, 2), wex::position_item(11, 5)),
        "Test diagnostic message"));

    wex::set_lsp_diagnostics(stc, diagnostics);
  }

  SECTION("set_lsp_hover")
  {
    wex::set_lsp_hover(
      stc,
      new wex::hover_item(wex::position_item(10, 2), "Test hover contents"));
  }

  SECTION("set_lsp_on_type")
  {
    auto* items = new wex::on_type_formatting_item_t;

    items->emplace_back(
      wex::on_type_formatting_item(
        wex::range_item(wex::position_item(12, 2), wex::position_item(12, 5)),
        "xyz"));
    items->emplace_back(
      wex::on_type_formatting_item(
        wex::range_item(wex::position_item(10, 2), wex::position_item(11, 5)),
        "abc"));

    wex::set_lsp_on_type(stc, items);
  }

  SECTION("set_lsp_show_message")
  {
    wex::set_lsp_show_message(
      frame(),
      new wex::show_message_item(
        wex::show_message_item::INFO,
        "Test show message"));
  }
}
