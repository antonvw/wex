////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/ctags.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExCTags")
{
  REQUIRE( wxExCTags(GetFrame()).Find("wxExTestApp") ); // tags default
  REQUIRE(!wxExCTags(GetFrame(), "xxx").Find("wxExTestApp") );
  
  REQUIRE(!wxExCTags(GetFrame(), "test-ctags").Find("xxxx") );
  REQUIRE( wxExCTags(GetFrame(), "test-ctags").Find("wxExTestApp") );
}
