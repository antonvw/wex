////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug-entry.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/debug-entry.h>

TEST_SUITE_BEGIN("wex::process");

TEST_CASE("wex::debug_entry")
{
  SUBCASE("default constructor")
  {
    REQUIRE(wex::debug_entry().get_commands().empty());
  }

  SUBCASE("constructor using xml")
  {
    pugi::xml_document doc;
    REQUIRE(doc.load_string("\
        <debug \
          name=\"gdb\" \
          extensions=\"*.cpp;*.h\" \
          flags=\"-XXX\"\
          regex-at-line=\"x.*yz\" \
          break-set=\"b\" \
          break-del=\"break delete\">\
        </debug>"));

    wex::debug_entry entry(doc.document_element());
    REQUIRE(entry.break_del() == "break delete");
    REQUIRE(entry.break_set() == "b");
    REQUIRE(entry.extensions() == "*.cpp;*.h");
    REQUIRE(entry.flags() == "-XXX");
    REQUIRE(entry.regex_stdout(wex::debug_entry::regex_t::AT_LINE) == "x.*yz");
    REQUIRE(entry.regex_stdout(wex::debug_entry::regex_t::PATH).empty());
  }
}

TEST_SUITE_END();
