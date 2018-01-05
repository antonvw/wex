////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-macros-mode.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi-macros-mode.h>
#include <wx/extension/variable.h>
#include "test.h"

TEST_CASE("wxExViMacrosMode")
{
  std::string expanded;
  wxExViMacrosMode mode;
  
  REQUIRE(!mode.IsRecording());
  REQUIRE(!mode.Expand(nullptr, wxExVariable("test"), expanded));
  REQUIRE( mode.Transition("x") == 0);
  REQUIRE( mode.String().empty());
}
