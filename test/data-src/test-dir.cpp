////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-dir.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/dir.h>
#include <wex/test/test.h>

TEST_CASE("wex::data::dir")
{
  SECTION("constructor")
  {
    wex::data::dir         dir;
    wex::data::dir::type_t t;
    t.set(wex::data::dir::FILES);

    REQUIRE(dir.dir_spec().empty());
    REQUIRE(dir.dir_spec("xx").dir_spec() == "xx");
    REQUIRE(dir.file_spec().empty());
    REQUIRE(dir.file_spec("yy").file_spec() == "yy");
    REQUIRE(!dir.file_spec("yy").is_regex());
    REQUIRE(dir.file_spec("yy", true).is_regex());
    REQUIRE(dir.find_replace_data() == nullptr);
    REQUIRE(dir.max_matches() == -1);
    REQUIRE(dir.max_matches(3).max_matches() == 3);
    REQUIRE(dir.type().test(wex::data::dir::FILES));
    REQUIRE(dir.type(t).type().test(wex::data::dir::FILES));
    REQUIRE(dir.vcs() == nullptr);
  }

  SECTION("type")
  {
    REQUIRE(wex::data::dir::type_t_def().test(wex::data::dir::FILES));
    REQUIRE(wex::data::dir::type_t_def().test(wex::data::dir::DIRS));
    REQUIRE(!wex::data::dir::type_t_def().test(wex::data::dir::HIDDEN));
    REQUIRE(wex::data::dir::type_t_def().test(wex::data::dir::RECURSIVE));
  }
}
