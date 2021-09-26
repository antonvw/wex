////////////////////////////////////////////////////////////////////////////////
// Name:      test-macros.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/macro-mode.h>
#include <wex/vi/macros.h>
#include <wex/vi/vi.h>

#include "test.h"

#define ESC "\x1b"

TEST_SUITE_BEGIN("wex::vi");

TEST_CASE("wex::macros" * doctest::may_fail())
{
  auto* stc = get_stc();
  auto* vi  = new wex::vi(stc);

  SUBCASE("constructor")
  {
    wex::macros macros;
    REQUIRE(macros.get_abbreviations().empty());
    REQUIRE(!macros.path().empty());
    REQUIRE(macros.size() == 0);
    REQUIRE(!macros.is_modified());
    REQUIRE(!macros.is_recorded("a"));
    REQUIRE(!macros.is_recorded_macro("a"));
    REQUIRE(!macros.mode().is_recording());
  }

  wex::macros macros;
  REQUIRE(macros.load_document());

  SUBCASE("load")
  {
    REQUIRE(macros.size() > 0);
    REQUIRE(!macros.is_recorded("x"));
    REQUIRE(!macros.get_keys_map().empty());
  }

  SUBCASE("record and playback")
  {
    vi->mode().escape();
    vi->get_stc()->set_text("hello");
    REQUIRE(vi->mode().is_command());
    REQUIRE(macros.mode().transition("qa") == 2);
    REQUIRE(!macros.is_modified());
    REQUIRE(macros.mode().is_recording());
    REQUIRE(macros.mode().get_macro() == "a");

    REQUIRE(macros.mode().transition("q") == 1);
    REQUIRE(!macros.mode().is_recording());
    REQUIRE(!macros.is_modified());
    REQUIRE(!macros.is_recorded("a")); // still no macro
    REQUIRE(!macros.is_recorded_macro("a"));
    REQUIRE(macros.mode().get_macro().empty());

    REQUIRE(macros.mode().transition("qa") == 2);
    REQUIRE(macros.mode().get_macro() == "a");
    REQUIRE(macros.record("a"));
    REQUIRE(macros.record("test"));
    REQUIRE(macros.record(ESC));
    REQUIRE(macros.mode().transition("q") == 1);
    REQUIRE(!macros.is_modified());
    REQUIRE(!macros.mode().is_recording());
    REQUIRE(macros.is_recorded("a"));
    REQUIRE(macros.starts_with("a"));
    REQUIRE(macros.is_recorded_macro("a"));
    REQUIRE(macros.mode().get_macro() == "a");
    REQUIRE(macros.find("a").front() == "a");
    REQUIRE(!macros.is_recorded_macro("d"));

    vi->get_stc()->set_text("");
    REQUIRE(vi->mode().is_command());
    REQUIRE(macros.mode().transition("@a", vi) == 2);
    REQUIRE(vi->get_stc()->get_text() == "test");

    vi->get_stc()->set_text("");
    REQUIRE(!macros.mode().transition("@a", vi, true, 0));
    REQUIRE(!macros.mode().transition("@a", vi, true, -8));
    REQUIRE(vi->get_stc()->get_text().find("test") == std::string::npos);
    REQUIRE(macros.mode().transition("@a", vi, true, 10));
    REQUIRE(
      vi->get_stc()->get_text().find("testtesttesttest") != std::string::npos);

    REQUIRE(macros.mode().transition("@b", vi) == 2);
    REQUIRE(!macros.get().empty());
  }

  SUBCASE("keysmap") { macros.set_key_map("4", "www"); }

  SUBCASE("append")
  {
    macros.mode().transition("qA", vi);
    macros.record("w");
    macros.record("/test");
    macros.mode().transition("q", vi);

    REQUIRE(!macros.is_recorded("A"));
    REQUIRE(macros.is_recorded("a"));
  }

  SUBCASE("recursive")
  {
    macros.mode().transition("qA", vi);
    macros.record("@");
    macros.record("a");
    macros.mode().transition("q", vi);

    REQUIRE(macros.mode().transition("@a", vi));
  }

  SUBCASE("builtin variables")
  {
    for (auto& builtin : get_builtin_variables())
    {
      CAPTURE(builtin);
      REQUIRE(macros.mode().transition("@" + builtin + "@", vi));
    }

    std::string expanded;

    REQUIRE(!macros.mode().expand(vi, wex::variable(), expanded));
    REQUIRE(!macros.mode().expand(vi, wex::variable("x"), expanded));
  }

  SUBCASE("environment variables")
  {
#ifdef __UNIX__
    for (auto& env : std::vector<std::string>{"HOME", "PWD"})
    {
      REQUIRE(macros.mode().transition("@" + env, vi));
    }
#endif
  }

  // Test input macro variables (requires input).
  // Test template macro variables (requires input).

  SUBCASE("save")
  {
    macros.set_abbreviation("TEST", "document is_modified");
    REQUIRE(macros.is_modified());
    REQUIRE(macros.save_document());
    REQUIRE(!macros.is_modified());

    // A second save is not necessary.
    REQUIRE(!macros.save_document());
  }

  SUBCASE("registers")
  {
    REQUIRE(!macros.get_register('a').empty());
    REQUIRE(!macros.get_registers().empty());
    REQUIRE(macros.set_register('z', "hello z"));
    REQUIRE(!macros.find("z").empty());
    REQUIRE(macros.get_register('z') == "hello z");
    REQUIRE(macros.set_register('Z', " and more"));
    REQUIRE(macros.get_register('Z').empty());
    REQUIRE(macros.get_register('z') == "hello z and more");
    REQUIRE(!macros.set_register('\x05', "hello z"));
    REQUIRE(macros.set_register('*', "clipboard"));
    REQUIRE(macros.set_register('_', "blackhole"));
    REQUIRE(macros.get_register('_').empty());
  }

  SUBCASE("abbreviations")
  {
    for (auto& abbrev : get_abbreviations())
    {
      macros.set_abbreviation(abbrev.first, abbrev.second);

      const auto& it = macros.get_abbreviations().find(abbrev.first);

      if (it != macros.get_abbreviations().end())
      {
        REQUIRE(abbrev.second == it->second);
      }
    }

    REQUIRE(macros.get_abbreviations().size() > get_abbreviations().size());
    REQUIRE(macros.is_modified());
    REQUIRE(macros.save_document());
  }
}

TEST_SUITE_END();
