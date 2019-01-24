////////////////////////////////////////////////////////////////////////////////
// Name:      test-log.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/log.h>
#include "../test.h"

TEST_CASE( "wex::log" ) 
{
  SUBCASE("default constructor")
  {
    REQUIRE( wex::log().get().empty());
    wex::log() << "default constructor";
  }
  
  SUBCASE("other constructors")
  {
    std::stringstream ss;
    ss << "the great white";

    wex::log(wex::log::DEBUG) << "level debug";
    wex::log("shark") << ss << "is white";

    wex::log log("shark", wex::log::DEBUG);
    log << std::stringstream("is hungry") << "eats" << 25 << "fish";

    REQUIRE( log.get().find("shark") == 0);
    REQUIRE( log.get().find("is hungry") != std::string::npos);
    REQUIRE( log.get().find(" eats ") != std::string::npos);
    REQUIRE( log.get().find("25") != std::string::npos);
  }
  
  SUBCASE("status")
  {
    wex::log::status() << get_testpath("test.h");
    wex::log::status(wex::log::status_t(wex::log::STAT_SYNC)) << "hello world";
    wex::log::status("hello") << "hello world";
  }
  
  SUBCASE("verbose")
  {
    wex::log::verbose() << get_testpath("test.h");
    wex::log::verbose("hello") << "hello world";
  }
}
