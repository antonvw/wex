////////////////////////////////////////////////////////////////////////////////
// Name:      test-file.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/log.h>
#include "../test.h"

TEST_CASE( "wxExLog" ) 
{
  std::stringstream ss;
  ss << "the great white";

  wxExLog() << "default constructor";
  wxExLog(LEVEL_DEBUG) << "level debug";
  wxExLog("shark") << ss << "is white";

  wxExLog log("shark", LEVEL_DEBUG);
  log << std::stringstream("is hungry") << "eats" << 25 << "fish";

  REQUIRE( log.Get().find("shark") == 0);
  REQUIRE( log.Get().find("is hungry") != std::string::npos);
  REQUIRE( log.Get().find(" eats ") != std::string::npos);
  REQUIRE( log.Get().find("25") != std::string::npos);
}
