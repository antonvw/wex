////////////////////////////////////////////////////////////////////////////////
// Name:      test-version.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/version.h>
#include "test.h"

TEST_CASE("wxExVersion")
{
  REQUIRE(!wxExVersionInfo().Get().empty());
  REQUIRE(!wxExGetVersionInfo().Get().empty());
}
