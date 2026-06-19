////////////////////////////////////////////////////////////////////////////////
// Name:      diagnostics-test.cpp
// Purpose:   Unit tests for LSP diagnostics storage
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/diagnostics.h>
#include <wex/test/test.h>

TEST_CASE("wex::lsp::diagnostics")
{
  wex::lsp::diagnostics diags;

  SECTION("add_and_get")
  {
    wex::diagnostic_item d;
    d.message               = "Error: undeclared variable";
    d.severity              = wex::severity_t::ERRORS;
    d.source                = "clang";
    d.code                  = "undeclared_var";
    d.range.start_line      = 5;
    d.range.start_character = 10;
    d.range.end_line        = 5;
    d.range.end_character   = 15;

    diags.add("file:///test.cpp", d);

    auto result = diags.get("file:///test.cpp");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].message == "Error: undeclared variable");
    REQUIRE(result[0].severity == wex::severity_t::ERRORS);
    REQUIRE(result[0].source == "clang");
  }

  SECTION("get_line_diagnostics")
  {
    wex::diagnostic_item d1, d2, d3;
    d1.message          = "Warning 1";
    d1.range.start_line = 10;
    d1.range.end_line   = 10;

    d2.message          = "Warning 2";
    d2.range.start_line = 10;
    d2.range.end_line   = 10;

    d3.message          = "Warning 3";
    d3.range.start_line = 15;
    d3.range.end_line   = 15;

    diags.add("file:///test.cpp", d1);
    diags.add("file:///test.cpp", d2);
    diags.add("file:///test.cpp", d3);

    auto line_diags = diags.get_line("file:///test.cpp", 10);
    REQUIRE(line_diags.size() == 2);
    REQUIRE(line_diags[0].message == "Warning 1");
    REQUIRE(line_diags[1].message == "Warning 2");

    auto line15_diags = diags.get_line("file:///test.cpp", 15);
    REQUIRE(line15_diags.size() == 1);
  }

  SECTION("clear_document")
  {
    wex::diagnostic_item d;
    d.message = "Test diagnostic";
    diags.add("file:///test.cpp", d);

    REQUIRE(diags.has("file:///test.cpp"));
    REQUIRE(diags.count() == 1);

    diags.clear("file:///test.cpp");

    REQUIRE(!diags.has("file:///test.cpp"));
    REQUIRE(diags.count() == 0);
    REQUIRE(diags.get("file:///test.cpp").empty());
  }

  SECTION("clear_all")
  {
    wex::diagnostic_item d;
    d.message = "Test";

    diags.add("file:///test1.cpp", d);
    diags.add("file:///test2.cpp", d);
    diags.add("file:///test3.cpp", d);

    REQUIRE(diags.count() == 3);

    diags.clear_all();

    REQUIRE(diags.count() == 0);
    REQUIRE(!diags.has("file:///test1.cpp"));
    REQUIRE(!diags.has("file:///test2.cpp"));
    REQUIRE(!diags.has("file:///test3.cpp"));
  }

  SECTION("get_uris")
  {
    wex::diagnostic_item d;
    d.message = "Test";

    diags.add("file:///project/main.cpp", d);
    diags.add("file:///project/utils.cpp", d);
    diags.add("file:///project/lib.cpp", d);

    auto uris = diags.get_uris();

    REQUIRE(uris.size() == 3);
    REQUIRE(
      std::find(uris.begin(), uris.end(), "file:///project/main.cpp") !=
      uris.end());
    REQUIRE(
      std::find(uris.begin(), uris.end(), "file:///project/utils.cpp") !=
      uris.end());
    REQUIRE(
      std::find(uris.begin(), uris.end(), "file:///project/lib.cpp") !=
      uris.end());
  }

  SECTION("multiple_documents")
  {
    wex::diagnostic_item d1, d2;
    d1.message = "Error in file1";
    d2.message = "Error in file2";

    diags.add("file:///file1.cpp", d1);
    diags.add("file:///file2.cpp", d2);

    REQUIRE(diags.get("file:///file1.cpp").size() == 1);
    REQUIRE(diags.get("file:///file2.cpp").size() == 1);
    REQUIRE(diags.get("file:///file1.cpp")[0].message == "Error in file1");
    REQUIRE(diags.get("file:///file2.cpp")[0].message == "Error in file2");
  }

  SECTION("severity_levels")
  {
    wex::diagnostic_item d_error, d_warning, d_info, d_hint;

    d_error.message  = "This is an error";
    d_error.severity = wex::severity_t::ERRORS;

    d_warning.message  = "This is a warning";
    d_warning.severity = wex::severity_t::WARNING;

    d_info.message  = "This is info";
    d_info.severity = wex::severity_t::INFO;

    d_hint.message  = "This is a hint";
    d_hint.severity = wex::severity_t::HINT;

    diags.add("file:///test.cpp", d_error);
    diags.add("file:///test.cpp", d_warning);
    diags.add("file:///test.cpp", d_info);
    diags.add("file:///test.cpp", d_hint);

    auto results = diags.get("file:///test.cpp");
    REQUIRE(results.size() == 4);
    REQUIRE(results[0].severity == wex::severity_t::ERRORS);
    REQUIRE(results[1].severity == wex::severity_t::WARNING);
    REQUIRE(results[2].severity == wex::severity_t::INFO);
    REQUIRE(results[3].severity == wex::severity_t::HINT);
  }

  SECTION("count_total_diagnostics")
  {
    wex::diagnostic_item d;
    d.message = "Diagnostic";

    REQUIRE(diags.count() == 0);

    diags.add("file:///test.cpp", d);
    REQUIRE(diags.count() == 1);

    diags.add("file:///test.cpp", d);
    REQUIRE(diags.count() == 2);

    diags.add("file:///other.cpp", d);
    REQUIRE(diags.count() == 3);
  }
}
