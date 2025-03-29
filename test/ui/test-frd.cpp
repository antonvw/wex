////////////////////////////////////////////////////////////////////////////////
// Name:      test-frd.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/find.h>
#include <wex/test/test.h>
#include <wex/ui/frd.h>

void check(const std::string& text, bool one, bool two, bool three, bool four)
{
  auto* frd = wex::find_replace_data::get();
  REQUIRE(frd->match(text, wex::data::find("xxx")) == one);
  REQUIRE(frd->match(text, wex::data::find("XXX")) == two);
  REQUIRE(frd->match(text, wex::data::find("xx")) == three);
  REQUIRE(frd->match(text, wex::data::find("XX")) == four);
}

TEST_CASE("wex::frd")
{
  auto* frd = wex::find_replace_data::get();

  SECTION("match")
  {
    const std::string text("xxx");

    REQUIRE(!frd->match(text, wex::data::find("")));

    frd->set_match_case(false);
    frd->set_match_word(false);
    check(text, true, true, true, true);

    frd->set_match_case(false);
    frd->set_match_word(true);
    check(text, true, true, false, false);

    frd->set_match_case(true);
    frd->set_match_word(false);
    check(text, true, false, true, false);

    frd->set_match_case(true);
    frd->set_match_word(true);
    check(text, true, false, false, false);

    frd->set_regex(true);
    check(text, true, true, false, false);

    REQUIRE(frd->match(text, wex::data::find("x+")));
    REQUIRE(frd->match(text, wex::data::find("X+")));
  }

  SECTION("get-set")
  {
    frd->set_regex(true);
    frd->set_find_string("find[0-9]");
    REQUIRE(frd->is_regex());
    REQUIRE(!frd->get_find_strings().empty());
    REQUIRE(frd->get_find_string() == "find[0-9]");

    const wex::ex_commandline_input::values_t l{"find3", "find4", "find5"};
    frd->set_find_strings(l);
    REQUIRE(frd->get_find_string() == "find3");

    frd->set_replace_string("replace1");
    frd->set_replace_string("replace2");
    frd->set_replace_string("replace[0-9]");
    REQUIRE(!frd->get_replace_strings().empty());
    REQUIRE(frd->get_replace_string() == "replace[0-9]");

    frd->set_replace_strings(l);
    REQUIRE(frd->get_find_string() == "find3");
    REQUIRE(frd->get_replace_string() == "find3");

    const wex::ex_commandline_input::values_t e;
    frd->set_find_strings(e);
    frd->set_replace_strings(e);
    REQUIRE(frd->get_find_strings().empty());
    REQUIRE(frd->get_find_string().empty());
    REQUIRE(frd->get_replace_strings().empty());
    REQUIRE(frd->get_replace_string().empty());
  }

  // take care we end with valid regex
  frd->set_find_string("find[0-9]");
  frd->set_regex(true);
  REQUIRE(frd->is_regex());
}
