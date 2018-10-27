////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/ctags.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::ctags")
{
  wex::stc_data data;

  SUBCASE("tags default")
  {
    wex::ex* ex = &GetSTC()->GetVi();

    REQUIRE( wex::ctags(ex).Find("wxExTestApp") );
    REQUIRE( wex::ctags(GetFrame()).Find("wxExTestApp") );
    REQUIRE( wex::ctags(ex).AutoComplete("wxExTest") == "wxExTestApp");

    wex::ctags_entry current;
    wex::ctags_entry filter;
    REQUIRE( wex::ctags(ex).Find("wxExTestApp", current, filter));
    REQUIRE( current.Kind() == "c" );
  }

  SUBCASE("tags non-existing file")
  {
    data.CTagsFileName("xxx");
    wex::stc* stc = new wex::stc(std::string("test"), data);
    AddPane(GetFrame(), stc);
    wex::ex* ex = &stc->GetVi();

    REQUIRE(!wex::ctags(ex).Find("wxExTestApp") );
  }
  
  SUBCASE("tags own file")
  {
    data.CTagsFileName("test-ctags");
    wex::stc* stc = new wex::stc(std::string("test"), data);
    AddPane(GetFrame(), stc);
    wex::ex* ex = &stc->GetVi();

    REQUIRE(!wex::ctags(ex).Find("") );
    REQUIRE(!wex::ctags(ex).Next() );
    REQUIRE(!wex::ctags(ex).Previous() );
    REQUIRE(!wex::ctags(ex).Find("xxxx") );
    REQUIRE( wex::ctags(ex).Find("wxExTestApp") );
    REQUIRE(!wex::ctags(ex).Next() );
    REQUIRE( wex::ctags(ex).Separator() != ' ');
  }
}
