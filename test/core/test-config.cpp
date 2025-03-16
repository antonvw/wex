////////////////////////////////////////////////////////////////////////////////
// Name:      test-config.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wx/font.h>

#include <wex/test/test.h>

TEST_CASE("wex::config")
{
  SUBCASE("default constructor")
  {
    REQUIRE(wex::config().get("x") == "x");
    REQUIRE(!wex::config().is_child());
    REQUIRE(!wex::config().child_start());
    REQUIRE(!wex::config().child_end());
    REQUIRE(wex::config::store_is_active());
  }

  SUBCASE("static")
  {
    REQUIRE(!wex::config::on_exit());

    REQUIRE(wex::config::store_is_active());

    REQUIRE(!wex::config::on_init());

    REQUIRE(wex::config::store_is_active());

    wex::config::discard();

    const auto& p(wex::config::path());

    REQUIRE(!p.empty());

    wex::config::set_path(p);
  }

  SUBCASE("dir")
  {
#ifdef __WXMSW__
    REQUIRE(!wex::config::dir().empty());
#else
    REQUIRE(wex::config::dir().string().contains(".config"));
#endif
  }

  SUBCASE("getters")
  {
    const wex::config::statusbar_t sb{
      {"one", {"normal"}, 2},
      {"two", {"flat"}, 4}};

    REQUIRE(wex::config("x").get(4) == 4);
    REQUIRE(wex::config("xcvb").get(4) == 4);
    REQUIRE(!wex::config("xcvb").exists());
    REQUIRE(wex::config("xcvb").empty());
    REQUIRE(wex::config("x").item() == "x");
    REQUIRE(wex::config("x").get("space") == "space");
    REQUIRE(wex::config("l").get(wex::config::strings_t{}).empty());
    REQUIRE(wex::config("l").get_first_of().empty());
    REQUIRE(wex::config("l").get_first_of("k") == "k");
    REQUIRE(wex::config("l").empty());
    REQUIRE(!wex::config("m").get(wex::config::strings_t{"one"}).empty());

    REQUIRE(std::get<0>(wex::config("sb").get(sb)[0]) == "one");
    REQUIRE(std::get<1>(wex::config("sb").get(sb)[0]).front() == "normal");
    REQUIRE(std::get<2>(wex::config("sb").get(sb)[0]) == 2);
    REQUIRE(std::get<0>(wex::config("sb").get(sb)[1]) == "two");
    REQUIRE(std::get<1>(wex::config("sb").get(sb)[1]).front() == "flat");
    REQUIRE(std::get<2>(wex::config("sb").get(sb)[1]) == 4);
    REQUIRE(wex::config("sbg").get(wex::config::statusbar_t{}).empty());

    wex::config::save();
  }

  SUBCASE("get-set-first-of")
  {
    REQUIRE(wex::config("l").get_first_of("l:0") == "");
    REQUIRE(wex::config("l").get_first_of("l:1") == "l");
    REQUIRE(wex::config("l").get_first_of("l:2") == "l:2");
    REQUIRE(wex::config("l").get_first_of("l:1000") == "l:1000");

    REQUIRE(wex::config("o").set_first_of("x:0") == "x:0");
    REQUIRE(wex::config("o").set_first_of("y:0") == "y:0");
    REQUIRE(wex::config("o").set_first_of("z:0") == "z:0");
    REQUIRE(wex::config("o").get_first_of() == "");

    REQUIRE(wex::config("m").set_first_of("one") == "one");
    REQUIRE(wex::config("m").set_first_of("two") == "two");
    REQUIRE(wex::config("m").set_first_of("xxx:1") == "xxx:1");
    REQUIRE(wex::config("m").get(wex::config::strings_t{}).size() == 3);
    REQUIRE(wex::config("m").get_first_of() == "xxx");
  }

  SUBCASE("setters")
  {
    wex::config("y").set(4);
    REQUIRE(wex::config("y").exists());
    REQUIRE(wex::config("y").get(0) == 4);

    wex::config("y").set("WW");
    REQUIRE(wex::config("y").exists());
    REQUIRE(!wex::config("y").empty());
    REQUIRE(wex::config("y").get("zzzz") == "WW");
    REQUIRE(wex::config("y").item("z").item() == "z");

    wex::config("list_items").set({"1", "2", "3"});
    REQUIRE(
      wex::config("list_items").get(wex::config::strings_t{}).front() == "1");

    wex::config("list_items_sel").set({"1:0", "2:1", "3:0"});
    REQUIRE(
      wex::config("list_items_sel").get(wex::config::strings_t{}).front() ==
      "1:0");
    REQUIRE(wex::config("list_items_sel").get_first_of() == "2");

    wex::config("y").erase();
    REQUIRE(!wex::config("y").exists());

    wex::config("stc.Default font")
      .set(wxFont(
        12,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL));

    wex::config("stc.Cursor line").set(true);

    REQUIRE(wex::config("stc.Print flags").get(3) == 3);

    const wex::config::statusbar_t sb{
      {"three", {"normal", "flat"}, 2},
      {"four", {"flat"}, 4}};

    REQUIRE(!std::get<0>(wex::config("sbs").get(sb)[0]).empty());

    wex::config("sbs").set(sb);
    REQUIRE(std::get<0>(wex::config("sbs").get(sb)[0]) == "three");
    REQUIRE(std::get<2>(wex::config("sbs").get(sb)[0]) == 2);

    wex::config::save();
  }

  SUBCASE("constructor child")
  {
    wex::config c("parent", "child-x");
    REQUIRE(c.is_child());

    REQUIRE(c.children() == 0);

    c.item("child-x").set("x");
    REQUIRE(c.item("child-x").get("y") == "x");

    wex::config("parent", "child-y").set("y");
    REQUIRE(wex::config("parent.child-y").get("z") == "y");
  }

  SUBCASE("child")
  {
    wex::config c("child");

    REQUIRE(!c.child_end());
    REQUIRE(c.child_start());
    REQUIRE(!c.child_start());

    c.item("child-x").set(1);
    c.item("child-y").set(2);
    c.item("child-z").set(3);

    REQUIRE(c.children() == 3);
    REQUIRE(!c.item("child").exists());
    REQUIRE(c.item("child-x").get(99) == 1);
    REQUIRE(c.item("child-y").get(99) == 2);
    REQUIRE(c.item("child-z").get(99) == 3);

    REQUIRE(c.child_end());
    REQUIRE(!c.child_end());

    REQUIRE(c.item("child").exists());
    REQUIRE(c.item("child.child-x").get(99) == 1);
    REQUIRE(c.item("child.child-y").get(99) == 2);
    REQUIRE(c.item("child.child-z").get(99) == 3);
  }

  SUBCASE("dotted-item")
  {
    wex::config("number.v").set(8);
    REQUIRE(!wex::config("number.v").is_child());
    REQUIRE(!wex::config("number.v").is_child());
    REQUIRE(wex::config("number.v").exists());
    REQUIRE(wex::config("number.v").get(9) == 8);

    wex::config("vector.v").set(wex::config::ints_t{1, 2, 3});
    REQUIRE(wex::config("vector.v").get(wex::config::ints_t{}).size() == 3);
  }

  SUBCASE("hierarchy")
  {
    wex::config("world.asia.china.cities").set("bejing");
    wex::config("world.europe.netherlands.cities").set("amsterdam");
    wex::config("world.europe.netherlands.rivers").set("rijn");
    REQUIRE(!wex::config("world.europe.netherlands.rivers").is_child());
    REQUIRE(wex::config("world.europe.netherlands.rivers").exists());
    REQUIRE(wex::config("world.europe.netherlands").exists());
    REQUIRE(!wex::config("world.europe.netherlands.mountains").exists());
    REQUIRE(wex::config("world.europe.netherlands.rivers").get() == "rijn");
  }

  SUBCASE("toggle")
  {
    wex::config c("toggle");
    c.set(true);
    REQUIRE(c.get(true));

    c.toggle();
    REQUIRE(!c.get(true));

    c.toggle();
    REQUIRE(c.get(true));

    REQUIRE(!c.toggle());
    REQUIRE(c.toggle());
  }
}
