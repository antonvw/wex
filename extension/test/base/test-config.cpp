////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/util.h>
#include <wex/ex.h>
#include "../test.h"

TEST_CASE("wex::config")
{
  SUBCASE("dir")
  {
#ifdef __WXMSW__
    REQUIRE(!wex::config().dir().empty());
#else
    REQUIRE( wex::config().dir().find(".config") != std::string::npos);
#endif
  }
  
  SUBCASE("firstof")
  {
    wex::config("xxxx").firstof();
  }
  
  SUBCASE("firstof_write")
  {
    REQUIRE( wex::config("xxxx").firstof_write("zz") == "zz");
  }
  
  SUBCASE("getters")
  {
    REQUIRE( wex::config("x").get(4) == 4);
    REQUIRE( wex::config("xcvb").get(4) == 4);
    REQUIRE(!wex::config("xcvb").exists());
    REQUIRE( wex::config("xcvb").empty());
    REQUIRE( wex::config("x").item() == "x");
    REQUIRE( wex::config("x").get("space") == "space");
    
    REQUIRE( wex::config("x").get_list().size() == 0);
  }
  
  SUBCASE("setters")
  {
    wex::config("y").set(4);
    REQUIRE( wex::config("y").get(0) == 4);

    wex::config("y").set("WW");
    REQUIRE( wex::config("y").exists());
    REQUIRE(!wex::config("y").empty());

    REQUIRE( wex::config("y").get("zzzz") == "WW");
    REQUIRE( wex::config("y").item("z").item() == "z");

    std::list < std::string > l{"1","2","3"};
    wex::config("list_items").set(l);
    
    REQUIRE(wex::config("list_items").get().find("1") != std::string::npos);
    REQUIRE(wex::config("list_items").get().find("2") != std::string::npos);
    
    wex::config("y").erase();
    REQUIRE(!wex::config("y").exists());
  }
}
