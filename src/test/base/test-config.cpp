////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include "../test.h"

TEST_CASE("wex::config")
{
  SUBCASE("default constructor")
  {
    REQUIRE( wex::config().get("x") == "x");
    wex::config().set("y");
  }
  
  SUBCASE("dir")
  {
#ifdef __WXMSW__
    REQUIRE(!wex::config::dir().empty());
#else
    REQUIRE( wex::config::dir().find(".config") != std::string::npos);
#endif
  }
  
  SUBCASE("getters")
  {
    REQUIRE( wex::config("x").get(4) == 4);
    REQUIRE( wex::config("xcvb").get(4) == 4);
    REQUIRE(!wex::config("xcvb").exists());
    REQUIRE( wex::config("xcvb").empty());
    REQUIRE( wex::config("x").item() == "x");
    REQUIRE( wex::config("x").get("space") == "space");
    REQUIRE( wex::config("l").get(std::list<std::string>{}).empty());
    REQUIRE( wex::config("l").get_firstof().empty());
    REQUIRE( wex::config("l").empty());
    REQUIRE(!wex::config("m").get(std::list<std::string>{"one"}).empty());
  }
  
  SUBCASE("setters")
  {
    REQUIRE( wex::config("m").set_firstof("one") == "one");
    REQUIRE( wex::config("m").set_firstof("two") == "two");
    REQUIRE( wex::config("m").get(std::list<std::string>{}).size() == 2);
    REQUIRE( wex::config("m").get_firstof() == "two");

    wex::config("y").set(4);
    REQUIRE( wex::config("y").exists());
    REQUIRE( wex::config("y").get(0) == 4);

    wex::config("y").set("WW");
    REQUIRE( wex::config("y").exists());
    REQUIRE(!wex::config("y").empty());
    REQUIRE( wex::config("y").get("zzzz") == "WW");
    REQUIRE( wex::config("y").item("z").item() == "z");

    wex::config("list_items").set({"1","2","3"});
    REQUIRE( wex::config("list_items").get(std::list<std::string>{}).front() == "1");
    
    wex::config("y").erase();
    REQUIRE(!wex::config("y").exists());
  }
}
