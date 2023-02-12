////////////////////////////////////////////////////////////////////////////////
// Name:      test-macros.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/ex/ex.h>
#include <wex/ex/macro-mode.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>

#include "test.h"

TEST_CASE("wex::macros" * doctest::may_fail())
{
  auto* stc = get_stc();
  auto* ex  = new wex::ex(stc);

  wex::macros macros;

  SUBCASE("constructor")
  {
    REQUIRE(macros.get_abbreviations().empty());
    REQUIRE(!macros.path().empty());
    REQUIRE(macros.size() == 0);
    REQUIRE(macros.get_map().empty());
    REQUIRE(macros.get_variables().empty());
    REQUIRE(!macros.is_modified());
    REQUIRE(!macros.is_recorded("a"));
    REQUIRE(!macros.is_recorded_macro("a"));
    REQUIRE(!macros.mode().is_recording());
  }

  REQUIRE(macros.load_document());

  SUBCASE("abbreviations")
  {
    for (auto& abbrev : wex::test::get_abbreviations())
    {
      macros.set_abbreviation(abbrev.first, abbrev.second);

      const auto& it = macros.get_abbreviations().find(abbrev.first);

      if (it != macros.get_abbreviations().end())
      {
        REQUIRE(abbrev.second == it->second);
      }
    }

    REQUIRE(
      macros.get_abbreviations().size() >
      wex::test::get_abbreviations().size());
    REQUIRE(macros.is_modified());
    REQUIRE(macros.save_document());
  }

  SUBCASE("add-erase")
  {
    macros.mode().transition("qA", ex);
    macros.record("w");
    macros.record("/test");
    macros.mode().transition("q", ex);

    REQUIRE(!macros.is_recorded("A"));
    REQUIRE(macros.is_recorded("a"));
    REQUIRE(!macros.get_macro_commands("a").empty());
    REQUIRE(macros.erase());

    wex::log_none off;
    REQUIRE(!macros.is_recorded("a"));
    REQUIRE(!macros.erase());
  }

  SUBCASE("builtin-variables")
  {
    for (auto& builtin : get_builtin_variables())
    {
      CAPTURE(builtin);
      REQUIRE(macros.mode().transition("@" + builtin + "@", ex));
    }

    std::string expanded;

    wex::log_none off;
    REQUIRE(!macros.mode().expand(ex, wex::variable(), expanded));
    REQUIRE(!macros.mode().expand(ex, wex::variable("x"), expanded));
  }

  SUBCASE("environment-variables")
  {
#ifdef __UNIX__
    for (auto& env : std::vector<std::string>{"HOME", "PWD"})
    {
      REQUIRE(macros.mode().transition("@" + env, ex));
    }
#endif
  }

  SUBCASE("load")
  {
    REQUIRE(macros.size() > 0);
    REQUIRE(!macros.is_recorded("x"));
    REQUIRE(!macros.get_keys_map().empty());
  }

  SUBCASE("map")
  {
    macros.set_key_map("4", "www");
    macros.set_map("Z", "www");
    REQUIRE(!macros.get_map().empty());
    REQUIRE(macros.get_keys_map().find(4) != macros.get_keys_map().end());
    REQUIRE(macros.get_keys_map().find(444) == macros.get_keys_map().end());
  }

  SUBCASE("record-and-playback")
  {
    ex->get_stc()->set_text("hello");
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
    REQUIRE(macros.record(wex::esc()));
    REQUIRE(macros.mode().transition("q") == 1);
    REQUIRE(!macros.is_modified());
    REQUIRE(!macros.mode().is_recording());
    REQUIRE(macros.is_recorded("a"));
    REQUIRE(macros.starts_with("a"));
    REQUIRE(macros.is_recorded_macro("a"));
    REQUIRE(macros.mode().get_macro() == "a");
    REQUIRE(macros.find("a").front() == "a");
    REQUIRE(!macros.is_recorded_macro("d"));

    ex->get_stc()->set_text("");
    REQUIRE(macros.mode().transition("@a", ex) == 2);

    // the macro does not result in chaning stc text,
    // see playback in wex::ex
    ex->get_stc()->set_text("");
    REQUIRE(!macros.mode().transition("@a", ex, true, 0));
    REQUIRE(!macros.mode().transition("@a", ex, true, -8));
    REQUIRE(macros.mode().transition("@a", ex, true, 10));
    REQUIRE(macros.mode().transition("@b", ex) == 2);

    REQUIRE(!macros.get().empty());
  }

  SUBCASE("recursive")
  {
    macros.mode().transition("qA", ex);
    macros.record("@");
    macros.record("a");
    macros.mode().transition("q", ex);

    REQUIRE(macros.mode().transition("@a", ex));
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

    REQUIRE(!macros.save_macro("zZ"));
  }
}
