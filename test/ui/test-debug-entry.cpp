////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug-entry.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/ui/debug-entry.h>

TEST_CASE("wex::debug_entry")
{
  SUBCASE("default constructor")
  {
    wex::debug_entry entry;

    REQUIRE(entry.flags().empty());
    REQUIRE(entry.break_del().empty());
    REQUIRE(entry.break_set().empty());
    REQUIRE(entry.get_commands().empty());

    for (auto r = static_cast<int>(wex::debug_entry::regex_t::AT_LINE);
         r <= (int)wex::debug_entry::regex_t::VARIABLE_MULTI;
         r++)
    {
      REQUIRE(
        entry.regex_stdout(static_cast<wex::debug_entry::regex_t>(r)).empty());
    }
  }

  SUBCASE("constructor using xml")
  {
    pugi::xml_document doc;
    REQUIRE(doc.load_string("\
        <debug \
          name=\"gdb\" \
          extensions=\"*.cpp;*.h\" \
          flags=\"-XXX\"\
          regex-no-file-line=\"yyy\" \
          regex-exit=\"xxx\" \
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
    REQUIRE(
      entry.regex_stdout(wex::debug_entry::regex_t::AT_PATH_LINE).empty());
    REQUIRE(
      entry.regex_stdout(wex::debug_entry::regex_t::BREAKPOINT_NO_FILE_LINE) ==
      "yyy");
    REQUIRE(entry.regex_stdout(wex::debug_entry::regex_t::EXIT) == "xxx");
    REQUIRE(entry.regex_stdout(wex::debug_entry::regex_t::PATH).empty());
    REQUIRE(entry.regex_stdout(wex::debug_entry::regex_t::VARIABLE).empty());
    REQUIRE(
      entry.regex_stdout(wex::debug_entry::regex_t::VARIABLE_MULTI).empty());
  }
}
