////////////////////////////////////////////////////////////////////////////////
// Name:      test-log.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/log.h>
#include "../test.h"

TEST_CASE( "wex::log" ) 
{
  std::stringstream ss;
  ss << "the great white";

  wex::log() << "default constructor";
  wex::log(wex::LEVEL_DEBUG) << "level debug";
  wex::log("shark") << ss << "is white";

  wex::log log("shark", wex::LEVEL_DEBUG);
  log << std::stringstream("is hungry") << "eats" << 25 << "fish";

  REQUIRE( log.Get().find("shark") == 0);
  REQUIRE( log.Get().find("is hungry") != std::string::npos);
  REQUIRE( log.Get().find(" eats ") != std::string::npos);
  REQUIRE( log.Get().find("25") != std::string::npos);
}
