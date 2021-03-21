////////////////////////////////////////////////////////////////////////////////
// Name:      test-frd.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/frd.h>
#include <wex/frame.h>

TEST_CASE("wex::frd")
{
  wex::find_replace_data* frd = wex::find_replace_data::get();

  REQUIRE(frd != nullptr);

  frd->set_match_case(true);
  REQUIRE(frd->match_case());
  frd->set_match_word(true);
  REQUIRE(frd->match_word());
  frd->set_search_down(true);
  REQUIRE(frd->search_down());
  frd->set_regex(true);
  REQUIRE(frd->is_regex());

  frd->set_find_string("find1");
  frd->set_find_string("find2");
  frd->set_find_string("find[0-9]");
  REQUIRE(frd->is_regex());
  REQUIRE(!frd->get_find_strings().empty());
  REQUIRE(frd->get_find_string() == "find[0-9]");

  REQUIRE(frd->regex_search("some text find9 other text") == 10);
  REQUIRE(frd->regex_search("some text finda other text") < 0);

  const std::list<std::string> l{"find3", "find4", "find5"};
  frd->set_find_strings(l);
  REQUIRE(frd->get_find_string() == "find3");

  frd->set_replace_string("replace1");
  frd->set_replace_string("replace2");
  frd->set_replace_string("replace[0-9]");
  REQUIRE(!frd->get_replace_strings().empty());
  REQUIRE(frd->get_replace_string() == "replace[0-9]");

  REQUIRE(!frd->text_find().empty());
  REQUIRE(!frd->text_match_case().empty());
  REQUIRE(!frd->text_match_word().empty());
  REQUIRE(!frd->text_regex().empty());
  REQUIRE(!frd->text_replace_with().empty());
  REQUIRE(!frd->text_search_down().empty());

  frd->set_replace_strings(l);
  REQUIRE(frd->get_find_string() == "find3");
  REQUIRE(frd->get_replace_string() == "find3");

  const std::list<std::string> e;
  frd->set_find_strings(e);
  frd->set_replace_strings(e);
  REQUIRE(frd->get_find_strings().empty());
  REQUIRE(frd->get_find_string().empty());
  REQUIRE(frd->get_replace_strings().empty());
  REQUIRE(frd->get_replace_string().empty());

  frd->set_find_string("find[0-9]");
  frd->set_replace_string("xxx");
  std::string text("find1 find2 find3 find4");
  const int   res = frd->regex_replace(text);
  REQUIRE(res == 4);

  frd->set_find_string("find[0-9");
  REQUIRE(!frd->is_regex());
  frd->set_regex(true);
  REQUIRE(!frd->is_regex());
  // take care we end with valid regex
  frd->set_find_string("find[0-9]");
  frd->set_regex(true);
  REQUIRE(frd->is_regex());
}
